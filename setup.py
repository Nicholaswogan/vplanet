from skbuild import setup

# Read current code version
VERSION = open("VERSION", "r").read().split("\n")[0].strip()

# Vplanet suite of tools
vplanet_suite = [
    "vplot>=1.0.2",
    "vspace>=1.0.2",
    "bigplanet>=1.0.1",
    "multiplanet>=1.0.1",
]

setup(
    name="vplanet",
    author="Rory Barnes",
    author_email="rkb9@uw.edu",
    version=VERSION,
    url="https://github.com/VirtualPlanetaryLaboratory/vplanet",
    description="The virtual planet simulator",
    long_description=open("README.md", "r").read(),
    long_description_content_type="text/markdown",
    license="MIT",
    packages=["vplanet"],
    install_requires=vplanet_suite + ["astropy>=3.0", "numpy", "tqdm",],
    python_requires=">=3.6",
    include_package_data=True,
    zip_safe=False,
    entry_points={"console_scripts": ["vplanet=vplanet.wrapper:_entry_point"]},
    cmake_args=['-DSKBUILD=ON']
)
