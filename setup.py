from setuptools import setup, Extension
import os

def main():
	src_files = ["src/" + f for f in os.listdir("src") if f.endswith(".c")]
	with open("readme.md", "r") as f:
		long_description = f.read()
	setup(
		name="qamar",
		version="0.1.0",
		description="A lua binding for python",
		long_description=long_description,
		author="Aurora Zuoris",
		author_email="programming.aurora@gmail.com",
		ext_modules=[
			Extension("qamar", src_files,include_dirs=["src"] , libraries=["lua"])
		])

if __name__ == "__main__":
	main()
