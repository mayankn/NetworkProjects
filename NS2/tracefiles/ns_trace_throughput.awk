BEGIN {
	fromNode = from;
	toNode = to;
	lineCount = 0;
	totalBits = 0;
}
/^r/ && $3 == fromNode && $4 == toNode {
    totalBits += 8*$6;
    if ( lineCount == 0 ) {
    	timeBegin = $2; 
    	lineCount++;
    } 
    else {
    	timeEnd = $2;
    };
};
END {
	totalDuration = timeEnd-timeBegin;
	print "  - Throughput = "  totalBits/totalDuration/1e3 " kbps";
};
