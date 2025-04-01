import lldb;

def set_variable(debugger, command, result, internal_dict):
    args = command.split()
    if len(args) != 2:
        print("Usage: set_var <variable_name> <new_value>")
        return

    var_name, new_value = args
    frame = debugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetSelectedFrame()
    var = frame.FindVariable(var_name.strip())

    if var.IsValid():
        success = var.SetValueFromCString(new_value)
        if success:
            print(f"{var_name} updated to {new_value}")
        else:
            print(f"Failed to update {var_name}")
    else:
        print(f"Variable {var_name} not found.")

def print_variable(debugger, command, result, internal_dict):
    # Get stack frame
    frame = debugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetSelectedFrame()
    var = frame.FindVariable(command.strip())
    if var.IsValid():
        print(f"{command.strip()} = {var.GetValue()}")
    else:
        print(f"Variable {command.strip()} not found.")

    val = int(var.GetValue(), 10)
    print("L4 index: {}\n", (val >> 39) & 0x1FF)
    print("L3 index: {}\n", (val >> 30) & 0x1FF)
    print("L2 index: {}\n", (val >> 21) & 0x1FF)
    print("L1 index: {}\n", (val >> 12) & 0x1FF)
    print(f"{command.strip()} = {var.GetValue()}")

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand('command script add -f parse_pt.set_variable svar')
    debugger.HandleCommand('command script add -f parse_pt.print_variable vaidx')
    print("Use `print_var <var_name>` to print a variable.")
