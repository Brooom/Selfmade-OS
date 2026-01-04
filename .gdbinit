#Compiles the project and load the symbols into gdb and load the .elf file by qemu
define reload
    #Stop current running OS
    monitor stop
    #Compile project and exit if compilation fails
    shell make || exit $?
    #Restart OS
    monitor system_reset
    #Load new OS into qemu
    load build/kernel.elf
    #Load symbols into gdb
    file build/kernel.elf
end

#Restarts the qemu session from the start.
#This uses the same kernel.elf file. 
#So in case you compiled a new file and what to use this one use the command restartOS
define reset
    echo Reseting OS...\n
    monitor system_reset
end