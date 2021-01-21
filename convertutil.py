infile = open(input("enter inputfilename: "), "rb")
data = infile.read()
infile.close()

outfile = open(input("enter output filename: "), "w")
for num in data:
	outfile.write(hex(num)+",")
outfile.close()