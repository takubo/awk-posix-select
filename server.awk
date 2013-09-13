BEGIN {
	extension("./awk-select.so", "dlload")

	net[0] = "/inet/tcp/8080/0/0"
	net[1] = "/inet/tcp/8081/0/0"
	net[2] = "/inet/tcp/8082/0/0"
	net[3] = "/inet/tcp/8083/0/0"

print "###"
	print "can" |& "/inet/tcp/0/localhost/8080"
	print "can" |& "/inet/tcp/0/localhost/8081"
	print "can" |& "/inet/tcp/0/localhost/8082"
	print "can" |& "/inet/tcp/0/localhost/8083"
system("sleep 1")
print "###"
			#net[0] |& getline
print "###"
			#net[1] |& getline
			#net[2] |& getline
			#net[3] |& getline

print "###"
	#close("/inet/tcp/0/localhost/8080")
	#close("/inet/tcp/0/localhost/8081")
	#close("/inet/tcp/0/localhost/8082")
	#close("/inet/tcp/0/localhost/8083")
print "###"
	while (1) {
		for (i = 0; i < 4; i++) {
			can = can_read(net[i], 1500)
			#print can
			if (!can) continue
			#if (!can_read(net[0], 300)) continue
			print "can"

			net[i] |& getline
			print

			if ($0 == "C")      { close(net[i]) }
			else if ($0 == "E") { exit(1) }
			else if ($0 == "q") { close(net[i]); exit(0) }
		}
	}
}
