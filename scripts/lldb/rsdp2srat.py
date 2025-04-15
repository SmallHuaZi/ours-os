import lldb
import struct

def read_memory(process, addr, size):
    error = lldb.SBError()
    data = process.ReadMemory(addr, size, error)
    return data if not error.Fail() else None

def read_u8(process, addr):
    data = read_memory(process, addr, 1)
    return struct.unpack("<B", data)[0] if data else 0

def read_u32(process, addr):
    data = read_memory(process, addr, 4)
    return struct.unpack("<I", data)[0] if data else 0

def read_u64(process, addr):
    data = read_memory(process, addr, 8)
    return struct.unpack("<Q", data)[0] if data else 0

def find_root_table(process, rsdp_addr):
    revision = read_u8(process, rsdp_addr + 15)
    if revision >= 2:
        xsdt_addr = read_u64(process, rsdp_addr + 24)
        return (xsdt_addr, 'XSDT') if xsdt_addr else (0, '')
    else:
        rsdt_addr = read_u32(process, rsdp_addr + 16)
        return (rsdt_addr, 'RSDT') if rsdt_addr else (0, '')

def find_srat(process, root_addr, table_type):
    # 验证根表签名
    signature = read_memory(process, root_addr, 4)
    expected_sig = b'XSDT' if table_type == 'XSDT' else b'RSDT'
    if signature != expected_sig:
        print(f"无效的{expected_sig.decode()}签名: {signature}")
        return None
    
    # 确定条目参数
    entry_size = 8 if table_type == 'XSDT' else 4
    read_entry = read_u64 if table_type == 'XSDT' else lambda p,a: read_u32(p,a)
    
    length = read_u32(process, root_addr + 4)
    entries = (length - 36) // entry_size  # ACPI头固定36字节
    
    for i in range(entries):
        entry_addr = root_addr + 36 + i * entry_size
        table_addr = read_entry(process, entry_addr)
        if not table_addr:
            continue
        
        # 检查签名是否为SRAT
        sig = read_memory(process, table_addr, 4)
        if sig == b"SRAT":
            return table_addr
    return None

def print_srat_entry(process, base, offset):
    entry_type = read_u8(process, base + offset)
    entry_len = read_u8(process, base + offset + 1)
    
    if entry_type == 0:  # Processor Local APIC/SAPIC Affinity
        proximity = read_u32(process, base + offset + 4)
        print(f"[Processor] NUMA节点: {proximity}")
        
    elif entry_type == 1:  # Memory Affinity
        base_addr = read_u64(process, base + offset + 8)
        length = read_u64(process, base + offset + 16)
        print(f"[Memory] 基地址: 0x{base_addr:x}, 长度: 0x{length:x}")
        
    elif entry_type == 2:  # Processor Local x2APIC Affinity
        proximity = read_u32(process, base + offset + 8)
        print(f"[x2APIC] NUMA节点: {proximity}")
        
    else:
        print(f"未知条目类型 {entry_type}，长度 {entry_len}")

def rsdp2srat(debugger, command, result, internal_dict):
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    frame = process.GetSelectedThread().GetSelectedFrame()
    
    # 获取RSDP变量地址
    rsdp_var = frame.FindVariable(command.split()[0])
    rsdp_addr = rsdp_var.GetValueAsUnsigned()
    print(f"RSDP @ 0x{rsdp_addr:x}")
    
    # 验证RSDP签名
    if read_memory(process, rsdp_addr, 8) != b"RSD PTR ":
        print("无效的RSDP签名!")
        return
    
    # 定位根表（RSDT/XSDT）
    root_addr, table_type = find_root_table(process, rsdp_addr)
    if not root_addr:
        print("找不到有效的根表!")
        return
    print(f"发现{table_type} @ 0x{root_addr:x}")
    
    # 查找SRAT表
    srat_addr = find_srat(process, root_addr, table_type)
    if not srat_addr:
        print("未找到SRAT表")
        return
    
    # 解析SRAT表
    length = read_u32(process, srat_addr + 4)
    print(f"\nSRAT表 @ 0x{srat_addr:x} (长度: {length} 字节)")
    
    offset = 44  # 跳过ACPI头(36)+Reserved(8)
    while offset < length:
        print_srat_entry(process, srat_addr, offset)
        offset += read_u8(process, srat_addr + offset + 1)

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(f"command script add -f {__name__}.rsdp2srat rsdp2srat")
    print('"rsdp2srat" 命令已安装，用法: rsdp2srat <rsdp_var>')
