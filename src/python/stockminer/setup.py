from setuptools import setup, find_packages

setup(
    name='stockminer',
    version='0.0.1',
    author='stockminer',
    author_email='stockminer@stockminer.org',
    description='Stockminer Python Module',
    long_description_content_type="text/markdown",
    url='ssh+git//git@stockminer.world/home/git/python/stockminer.git',
    license='MIT',
	packages=find_packages(where="src"),
    package_dir={"": "src"},
    install_requires=['numpy'],
)
