BEGIN {
	dropPackets = 0;
	sentPackets = 0;
}
{
	action = $1;
	fromNode = $3;
	toNode = $4;
	flow_id = $8;
 
	if (fromNode==from && toNode==to && action == "+")
		sentPackets++;
	if (flow_id==flow && action == "d")
		dropPackets++;
}
END {
        printf("Packet Loss Rate: %f\n", (dropPackets/sentPackets)*1000);
}
