#!/bin/bash
val1=1.3
val2=1.1
> log.txt
while : 
do
    let countyou=0
    let countme=0
    let i=4
    let remainder=0
    while [ $i -lt 8 ]; do
        source compile.sh
        > me$i.txt
        > you$i.txt
        python server.py 123$i -TL 200 &
        sleep 3s
        if [ $remainder -eq 0 ]; then
            python client.py localhost 123$i -val $val1 run.sh >> me$i.txt 2>&1 &
        else
            python client.py localhost 123$i -val $val2 run.sh >> you$i.txt 2>&1 &
        fi
        sleep 3s
        if [ $remainder -eq 0 ]; then
            python client.py localhost 123$i -mode NONE -val $val2 run.sh >> you$i.txt 2>&1 &
        else
            python client.py localhost 123$i -mode NONE -val $val1 run.sh >> me$i.txt 2>&1 &
        fi
        while :
        do 
            line1=`grep 'LOSE' me$i.txt`
            line2=`grep 'LOSE' you$i.txt`
            line3=`grep 'WIN' me$i.txt`
            line4=`grep 'WIN' you$i.txt`
            if [ -z "$line2" ] && [ -z "$line3" ]; then
                sleep 3s
            else
                echo first WINS
                let countme=countme+1
                break
            fi
            if [ -z "$line1" ] && [ -z "$line4" ]; then
                sleep 3s
            else
                echo second WINS
                let countyou=countyou+1
                break
            fi 
        done
        source kill_python.sh
        let i=i+1
        let remainder=($i % 2)
        echo countme $countme
        echo countyou $countyou
        if [ $countme -gt 4 ]; then
            break;
        elif [ $countyou -gt 4 ]; then
            break;
        fi
    done
    > output.txt
    source update_value.sh $val1 $val2 $countme $countyou >> output.txt
    val1=`grep 'val1' output.txt | awk '{print$2}'`
    val2=`grep 'val2' output.txt | awk '{print$2}'`
    echo $val1 >> log.txt
    echo $val2 >> log.txt
    echo $val1
    echo $val2
done
echo DONE
