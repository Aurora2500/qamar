from distutils.core import setup, Extension

def main():
	setup(
		name="qamar",
		version="0.1.0",
		description="A lua binding for python",
		author="Aurora Zuoris",
		author_email="programming.aurora@gmail.com",
		ext_modules=[
			Extension("qamar", ["src/qamar.c"], libraries=["lua"])
		])

if __name__ == "__main__":
	main()
