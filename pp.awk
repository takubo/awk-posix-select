BEGIN {
	extension("./awk-select.so", "dlload")

	c = "./rep"

	print "" |& c
			c |& getline
	print 

	while (1) {
	print can
			can = can_read(c, 1000)
			#print can
			if (!can) continue
			#if (!can_read(net[0], 300)) continue
			print "can"
			c |& getline
			print
	}
}
