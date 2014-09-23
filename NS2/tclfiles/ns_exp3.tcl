#Create a simulator object
set ns [new Simulator]

#Open the trace file (before you start the experiment!)
set tf [open ../tracefiles/exp3_output.tr w]
$ns trace-all $tf

#Define a 'finish' procedure
proc finish {} {
	global ns tf
        #Close the trace file
        close $tf
        exit 0
}

#Create six nodes for dumbell network topology
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
set n5 [$ns node]
set n6 [$ns node]

#Create links between the nodes
$ns duplex-link $n1 $n2 10Mb 10ms [lindex $argv 2]
$ns duplex-link $n5 $n2 10Mb 10ms [lindex $argv 2]
$ns duplex-link $n2 $n3 10Mb 10ms [lindex $argv 2]
$ns duplex-link $n3 $n4 10Mb 10ms [lindex $argv 2]
$ns duplex-link $n3 $n6 10Mb 10ms [lindex $argv 2]

#Set queue size for the bottleneck link (Node2-Node3) to 10
#$ns queue-limit $n2 $n3 10

#Setup a UDP Connection
#UDP Agent for sending
set udpAgent [new Agent/UDP]
$ns attach-agent $n5 $udpAgent
#UDP Sink for receiving
set udpSink [new Agent/Null]
$ns attach-agent $n6 $udpSink
$ns connect $udpAgent $udpSink
$udpAgent set fid_ 2

#Setup a CBR over UDP connection
set cbrFlow [new Application/Traffic/CBR]
$cbrFlow set type_ CBR
$cbrFlow set packet_size_ 1000
$cbrFlow set rate_ 5mb
$cbrFlow attach-agent $udpAgent

#Setup a TCP connection
set tcpAgent [new Agent/[lindex $argv 0]]
$tcpAgent set class_ 2
$ns attach-agent $n1 $tcpAgent
set tcpSink [new Agent/[lindex $argv 1]]
$ns attach-agent $n4 $tcpSink
$ns connect $tcpAgent $tcpSink
$tcpAgent set fid_ 1
$tcpAgent set window_ 10000

#Setup a FTP over TCP connection
set ftpApp [new Application/FTP]
$ftpApp attach-agent $tcpAgent
$ftpApp set type_ FTP

#Schedule events for the CBR and FTP agents
$ns at 1.0 "$ftpApp start"
$ns at 150.0 "$cbrFlow start"
$ns at 300.0 "$cbrFlow stop"
$ns at 300.0 "$ftpApp stop"

#Call the finish procedure after 5 seconds of simulation time
$ns at 300.5 "finish"

#Run the simulation
$ns run
