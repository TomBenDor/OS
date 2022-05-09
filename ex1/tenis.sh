#!/bin/bash
#Tom Ben-Dor

currentZone=0
P1Points=50
P2Points=50

printState () {
    echo " Player 1: ${P1Points}         Player 2: ${P2Points} "
    echo " --------------------------------- "
    echo " |       |       #       |       | "
    case $currentZone in
        -3)
            echo "O|       |       #       |       | "
            ;;
        -2)
            echo " |   O   |       #       |       | "
            ;;
        -1)
            echo " |       |   O   #       |       | "
            ;;
        0)
            echo " |       |       O       |       | "
            ;;
        1)
            echo " |       |       #   O   |       | "
            ;;
        2)
            echo " |       |       #       |   O   | "
            ;;
        3)
            echo " |       |       #       |       |O"
            ;;
    esac


    echo " |       |       #       |       | "
    echo " --------------------------------- "
}

main() {
    printState
    P1Move=0
    P2Move=0
    while [ 1 ]; do
        echo "PLAYER 1 PICK A NUMBER: "
        read -s P1Move
        if ! [[ $P1Move =~ ^[0-9]+$ ]] || [ $P1Move -gt $P1Points ] || [ $P1Move -lt 0 ]; then
            echo "NOT A VALID MOVE !"
            continue
        fi
        echo "PLAYER 2 PICK A NUMBER: "
        read -s P2Move
        if ! [[ $P2Move =~ ^[0-9]+$ ]] || [ $P2Move -gt $P2Points ] || [ $P2Move -lt 0 ]; then
            echo "NOT A VALID MOVE !"
            continue
        fi
        if [ $P1Move -gt $P2Move ]; then
            if [ $currentZone -lt 1 ]; then
                currentZone=1
            else
                ((currentZone=currentZone+1))
            fi
        else
            if [ $P2Move -gt $P1Move ]; then
                if [ $currentZone -gt -1 ]; then
                    currentZone=-1
                else
                    ((currentZone=currentZone-1))
                fi
            fi
        fi
        ((P1Points=P1Points-P1Move))
        ((P2Points=P2Points-P2Move))
        printState
        echo -e "       Player 1 played: ${P1Move}\n       Player 2 played: ${P2Move}\n\n"

        if [ $currentZone -eq -3 ]; then
            echo "PLAYER 2 WINS !"
            break
        fi
        if [ $currentZone -eq 3 ]; then
            echo "PLAYER 1 WINS !"
            break
        fi

        if [ $P1Points -eq 0 ] && [ $P2Points -gt 0 ]; then
            echo "PLAYER 2 WINS !"
            break
        fi
        if [ $P2Points -eq 0 ] && [ $P1Points -gt 0 ]; then
            echo "PLAYER 1 WINS !"
            break
        fi

        if [ $P1Points -eq 0 ] && [ $P2Points -eq 0 ]; then
            if [ $currentZone -lt 0 ]; then
                echo "PLAYER 2 WINS !"
                break
            fi
            if [ $currentZone -gt 0 ]; then
                echo "PLAYER 1 WINS !"
                break
            fi

            echo "IT'S A DRAW !"
            break
        fi

    done

}

main
