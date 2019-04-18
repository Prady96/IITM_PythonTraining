## get the file listing by unix file system
import glob

def create_files():

	files = [
		"1.jpg",
		"2.jpg",
		"3.jpg",
	]
	for afile in files:
		open(afile, 'wb').close()

def main():
	print("------- files list -------")
	create_files()
	files = glob.glob("*.jpg")

	for afile in files:
		print(afile)
	return 0

if __name__ == "__main__":
	main()