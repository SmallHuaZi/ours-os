rm -f ours.*

./obit create -t kpkg -o ours.kpkg
./obit kpkg -p ours.kpkg --add kernel_main -t elf -e -o mmod.elf

output=$(command cmp -i32:0 ours.kpkg kernel_main)
status=$1
if [ -n "$output" ]; then
    echo "The contents both the primary file and the OBI do not match."
    exit 1
fi

./obit create -f ours.kpkg -t cont -o ours.obi