BEGIN {
	fromNode = from;
	toNode = to;
	lineCount = 0;
	totalBits = 0;
	interval = 10;
	record_from = fromTime;
}
/^r/ && $3 == fromNode && $4 == toNode {
	if ($2 >= record_from) { 
		if ( $2 < (record_from + interval)) {
			totalBits += 8*$6;
			#print "line: " lineCount " ";
			if ( lineCount == 0 ) {
        			timeBegin = $2;
        			lineCount++;
    			}
			else {
				timeEnd = $2;
			}
		}
		else {
			lineCount = 0;
			record_from += interval;
                	duration = timeEnd - timeBegin;
                	throughput = (totalBits/duration)/1e3;
			totalBits = 0;
                	print record_from " " throughput " kbps";
		};
	}
};
END {
};
