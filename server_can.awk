BEGIN {
	extension("./awk-select.so", "dlload")

	net = "/inet/tcp/8080/0/0"

	while (1) {
		net |& getline

		print

		if ($0 == "C")      { close(net) }
		else if ($0 == "E") { exit(1) }
		else if ($0 == "q") { close(net); exit(0) }

		while (!can_read(net, 1000))
			print "#"
		#while (1) {
		#	#exc = has_exception(net, 1500)
		#	#if (exc) {print "has"; close(net); exit(0) }
		#	can = can_read(net, 1500)

		#	print can

		#	if (can) break
		#}
	}
}
