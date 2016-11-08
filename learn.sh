#!/bin/bash
val1=30
val2=20
> log.txt
while : 
do
    echo $val1 >> log.txt
    echo $val2 >> log.txt
    let countyou=0
    let countme=0
    let i=$1
    let j=($1 + 8)
    let remainder=0
    while [ $i -lt $j ]; do
        echo value of i is $i
        source compile.sh
        > me$i.txt
        > you$i.txt
        python server.py 123$i -TL 2000 &
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
            line1=`grep 'winner' me$i.txt`
            line2=`grep 'winner' you$i.txt`
            line3=`grep 'loser' me$i.txt`
            line4=`grep 'loser' you$i.txt`
            if [ -z "$line1" ]; then
                sleep 2s
            else
                echo first WINS
                score_me=`grep 'Scorewin' me$i.txt | awk '{print$2}'`
                score_you=`grep 'Scorelose' me$i.txt | awk '{print$2}'`
                echo $score_me
                echo $score_you
                let countme=($countme + $score_me)
                let countyou=($countyou + $score_you)
                break
            fi
            if [ -z "$line4" ]; then
                sleep 2s
            else
                echo first WINS
                score_me=`grep 'Scorewin' you$i.txt | awk '{print$2}'`
                score_you=`grep 'Scorelose' you$i.txt | awk '{print$2}'`
                echo $score_me
                echo $score_you
                let countme=($countme + $score_me)
                let countyou=($countyou + $score_you)
                break
            fi
            if [ -z "$line2" ]; then
                sleep 2s
            else
                echo second WINS
                score_you=`grep 'Scorewin' you$i.txt | awk '{print$2}'`
                score_me=`grep 'Scorelose' you$i.txt | awk '{print$2}'`
                echo $score_me
                echo $score_you
                let countme=($countme + $score_me)
                let countyou=($countyou + $score_you)
                break
            fi
            if [ -z "$line3" ]; then
                sleep 2s 
            else
                echo second WINS
                score_you=`grep 'Scorewin' me$i.txt | awk '{print$2}'`
                score_me=`grep 'Scorelose' me$i.txt | awk '{print$2}'`
                echo $score_me
                echo $score_you
                let countme=($countme + $score_me)
                let countyou=($countyou + $score_you)
                break
            fi
        done
        source kill_python.sh
        let i=i+1
        let remainder=($i % 2)
        echo countme $countme
        echo countyou $countyou
        if [ $countme -gt 120 ]; then
            break;
        elif [ $countyou -gt 120 ]; then
            break;
        fi
    done
    > output.txt
    source update_value.sh $val1 $val2 $countme $countyou >> output.txt
    val1=`grep 'val1' output.txt | awk '{print$2}'`
    val2=`grep 'val2' output.txt | awk '{print$2}'`
    echo $val1
    echo $val2
done
echo DONE
