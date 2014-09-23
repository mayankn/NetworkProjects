BEGIN {
   src = source;
   dst = destination;
   samples = 0;
   total_delay = 0;
   flow_id = $8;
}

/^\+/&&$9==src&&$10==dst&&$3==from{
  start_time[$11] = $2;
};

/^r/&&$9==dst&&$10==src&&$4==from{
 if (start_time[$11] > 0) {
   samples++ ;
   delay = $2 - start_time[$11];
   total_delay += delay;
    };
};
END{
  avg_delay = total_delay/samples;
  print "Average RTT is =" 1000 * avg_delay " ms";
};
