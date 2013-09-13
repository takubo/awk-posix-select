BEGIN {
	t = "0"
	extension("./awk-select.so", "dlload")
	print "" > t
	#print can_write(t, 1000)
	delete r
	r[0] = t
	#r["4#"] = "W"
	#r["@"] = "$"
	#r[1] = 1
	#r[2] = 2
	#r["3#"] = "CC"
	##r[3] = 3
	w[0] = t
	e[0] = t
	print select(r, w, e, 1000)
}

