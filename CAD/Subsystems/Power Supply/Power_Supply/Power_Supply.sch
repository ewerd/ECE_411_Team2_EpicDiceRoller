EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Power Supply"
Date "2021-11-09"
Rev "2"
Comp "Team 2"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Regulator_Switching:MC34063AD U1
U 1 1 618B2B8F
P 5950 3600
F 0 "U1" H 5950 4067 50  0000 C CNN
F 1 "MC34063AD" H 5950 3976 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 6000 3150 50  0001 L CNN
F 3 "http://www.onsemi.com/pub_link/Collateral/MC34063A-D.PDF" H 6450 3500 50  0001 C CNN
	1    5950 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 618B4184
P 5950 4100
F 0 "#PWR?" H 5950 3850 50  0001 C CNN
F 1 "GND" H 5955 3927 50  0000 C CNN
F 2 "" H 5950 4100 50  0001 C CNN
F 3 "" H 5950 4100 50  0001 C CNN
	1    5950 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 3800 5250 3800
$Comp
L power:GND #PWR?
U 1 1 618B5679
P 5250 4100
F 0 "#PWR?" H 5250 3850 50  0001 C CNN
F 1 "GND" H 5255 3927 50  0000 C CNN
F 2 "" H 5250 4100 50  0001 C CNN
F 3 "" H 5250 4100 50  0001 C CNN
	1    5250 4100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 618B6E66
P 4500 4100
F 0 "#PWR?" H 4500 3850 50  0001 C CNN
F 1 "GND" H 4505 3927 50  0000 C CNN
F 2 "" H 4500 4100 50  0001 C CNN
F 3 "" H 4500 4100 50  0001 C CNN
	1    4500 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 3400 5250 3400
Wire Wire Line
	4500 3400 4500 3800
Wire Wire Line
	4500 3400 4150 3400
Connection ~ 4500 3400
Text GLabel 4150 3400 0    50   Input ~ 0
BATT
$Comp
L Device:R R1
U 1 1 618B8597
P 5950 2950
F 0 "R1" V 5743 2950 50  0000 C CNN
F 1 "0.33Ohm" V 5834 2950 50  0000 C CNN
F 2 "" V 5880 2950 50  0001 C CNN
F 3 "~" H 5950 2950 50  0001 C CNN
	1    5950 2950
	0    1    1    0   
$EndComp
Wire Wire Line
	5250 3400 5250 2950
Wire Wire Line
	5250 2950 5800 2950
Connection ~ 5250 3400
Wire Wire Line
	5250 3400 4500 3400
Wire Wire Line
	6100 2950 6450 2950
Wire Wire Line
	6450 2950 6450 3400
Wire Wire Line
	6450 3600 6350 3600
Wire Wire Line
	6450 3500 6350 3500
Connection ~ 6450 3500
Wire Wire Line
	6450 3500 6450 3600
Wire Wire Line
	6450 3400 6350 3400
Connection ~ 6450 3400
Wire Wire Line
	6450 3400 6450 3500
$Comp
L Device:L L1
U 1 1 618BA459
P 7100 3800
F 0 "L1" V 7290 3800 50  0000 C CNN
F 1 "100uH" V 7199 3800 50  0000 C CNN
F 2 "" H 7100 3800 50  0001 C CNN
F 3 "~" H 7100 3800 50  0001 C CNN
	1    7100 3800
	0    -1   -1   0   
$EndComp
$Comp
L Diode:BZX55C3V0 D1
U 1 1 618BC3BC
P 6800 3950
F 0 "D1" V 6754 4030 50  0000 L CNN
F 1 "BZX55CXX" V 6845 4030 50  0000 L CNN
F 2 "DO-35" H 6800 3775 50  0001 C CNN
F 3 "https://diotec.com/tl_files/diotec/files/pdf/datasheets/bzx84c2v4.pdf" H 6800 3950 50  0001 C CNN
	1    6800 3950
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 618BDD36
P 6800 4250
F 0 "R2" H 6870 4296 50  0000 L CNN
F 1 "1kOhm" H 6870 4205 50  0000 L CNN
F 2 "" V 6730 4250 50  0001 C CNN
F 3 "~" H 6800 4250 50  0001 C CNN
	1    6800 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 618BE9A5
P 6800 4400
F 0 "#PWR?" H 6800 4150 50  0001 C CNN
F 1 "GND" H 6805 4227 50  0000 C CNN
F 2 "" H 6800 4400 50  0001 C CNN
F 3 "" H 6800 4400 50  0001 C CNN
	1    6800 4400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 618BF43B
P 7450 3950
F 0 "C3" H 7565 3996 50  0000 L CNN
F 1 "470uF" H 7565 3905 50  0000 L CNN
F 2 "" H 7488 3800 50  0001 C CNN
F 3 "~" H 7450 3950 50  0001 C CNN
	1    7450 3950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 618C02A3
P 5250 3950
F 0 "C1" H 5365 3996 50  0000 L CNN
F 1 "470pF" H 5365 3905 50  0000 L CNN
F 2 "" H 5288 3800 50  0001 C CNN
F 3 "~" H 5250 3950 50  0001 C CNN
	1    5250 3950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 618C0C5B
P 4500 3950
F 0 "C2" H 4615 3996 50  0000 L CNN
F 1 "100uF" H 4615 3905 50  0000 L CNN
F 2 "" H 4538 3800 50  0001 C CNN
F 3 "~" H 4500 3950 50  0001 C CNN
	1    4500 3950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 618C1D41
P 7450 4100
F 0 "#PWR?" H 7450 3850 50  0001 C CNN
F 1 "GND" H 7455 3927 50  0000 C CNN
F 2 "" H 7450 4100 50  0001 C CNN
F 3 "" H 7450 4100 50  0001 C CNN
	1    7450 4100
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT RV1
U 1 1 618C2705
P 6500 4750
F 0 "RV1" V 6385 4750 50  0000 C CNN
F 1 "5kOhm" V 6294 4750 50  0000 C CNN
F 2 "" H 6500 4750 50  0001 C CNN
F 3 "~" H 6500 4750 50  0001 C CNN
	1    6500 4750
	0    1    -1   0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 618C4366
P 6200 4800
F 0 "#PWR?" H 6200 4550 50  0001 C CNN
F 1 "GND" H 6205 4627 50  0000 C CNN
F 2 "" H 6200 4800 50  0001 C CNN
F 3 "" H 6200 4800 50  0001 C CNN
	1    6200 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 3800 6800 3800
Wire Wire Line
	6800 3800 6950 3800
Connection ~ 6800 3800
Wire Wire Line
	7250 3800 7450 3800
Wire Wire Line
	7450 3800 7900 3800
Wire Wire Line
	7900 3800 7900 4750
Wire Wire Line
	7900 4750 6650 4750
Connection ~ 7450 3800
Wire Wire Line
	6500 4600 6500 3900
Wire Wire Line
	6500 3900 6350 3900
Wire Wire Line
	6350 4750 6200 4750
Wire Wire Line
	6200 4750 6200 4800
Text GLabel 8150 3800 2    50   Input ~ 0
Vcc
Wire Wire Line
	7900 3800 8150 3800
Connection ~ 7900 3800
$EndSCHEMATC
