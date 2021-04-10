#!/bin/bash

if [ `pgrep button_process` > 0 ]; then
    echo "Killing button_process"
    kill `pgrep button_process`
fi

if [ `pgrep led_process` > 0 ]; then    
    echo "Killing led_process"
    kill `pgrep led_process`
fi
