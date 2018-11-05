reset
set xlabel 'N'
set ylabel 'ratio'
set title 'Simulation Pn'
set term png enhanced font 'Verdana,10'
set output 'pn.png'
set format x "%10.0f"
set xtics rotate by 45 right
 plot [:][:]'pn.txt' using 1:2 pt 2 ps 2 with points title 'simulation',\
 	'pn.txt' using 1:3 pt 7 ps 1 with points title 'expect',\
