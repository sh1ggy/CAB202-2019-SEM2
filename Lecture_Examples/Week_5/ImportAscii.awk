@load "ordchr"

{
	l = $0;
	len = length(l);

	for ( i = 1; i <= len; i++ ) {
		if (substr(l,i,1) != " ") {
			print (i-1) / 80.0, NR / 24.0;  
		}
	}
}
