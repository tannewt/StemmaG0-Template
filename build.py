import embedded.cpu
import embedded.cli
import embedded.microcontroller

import pathlib

async def build_stemmag0():
	mcu = embedded.microcontroller.get_mcus_from_string("STM32G030F6")[0]
	await mcu.generate_c_header("I2C1", pathlib.Path("build/i2c.h"))
	print(mcu)

if __name__ == "__main__":
	embedded.cli.run(build_stemmag0)
