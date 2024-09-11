set term qt 0
set title 'ADC Data Values'
set yrange [-20:20]
set xlabel 'time (s)'
set ylabel 'Volts'
plot 'captured_data.txt' using 1:2 with lines t 'channel 0', 'captured_data.txt' using 1:3 with lines t 'channel 1'

pause -1 "Hit any key to continue"

