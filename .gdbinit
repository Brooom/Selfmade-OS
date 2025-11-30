define restartOS
    echo Restarting OS...\n
    file build/kernel.elf
    target remote :1234
end

define resetOS
    echo Reseting OS...\n
    monitor system_reset
end