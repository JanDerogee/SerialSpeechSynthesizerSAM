# SSSSAM
Serial Speech Synthesizer SAM

This speech synthesizer is actually the SAM speech synthesizer in an ESP8266.
Where SAM was a software application for the C64, Atari or Apple computer, this implementation is embedded into the ESP8266. It acts like a serial device, it can be easily controlled using simple serial commands. Because of this it can works with any system independent of the amount of available RAM. Meaning that you can use SAM on a VIC-20.

If this device was to be compared with an existing device, it could be compared best to the "Votrax type 'n' talk". That device and this project both connect to the serial port and have enough to human readable text, no need for phonemes if you don't want to, you can make it say "hello world" just by sending it "hello world". How easy is that.

A long long time ago, there were a set of adventure games, written by Scott Adams, for the VIC-20. These games also send the text data to the serial port. Meaning that if you connected a "Votrax type 'n' talk" you could make your adventure game speak. Which was something awsome at the time but impossible for a youngster on a budget... and now that that youngster has grown up, it still can't do that because the "Votrax type 'n' talk" are highly priced collectors items.

Thankfully this project fills that gap.
Using cheap components and "open source" speech synthesis a simple speech synthesizer is created.
Making your 8-bit Commodore computer speak. By connecting the device to the user-port of your VIC-20, C64, C128 or PET computer. The device can be connected to a small (amplified) speaker or via a splitter cable the sound can be mixed with the regular audio of the computer. This way the sound comes from your monitors or TV speaker, making it sound very professional.

This project is heavily based on the good work of:

Sebastian Macke     https://github.com/s-macke/SAM

Earle F. Philhower  https://github.com/earlephilhower/ESP8266Audio

So that all I needed to do was to create an interface around it, allowing for setting control and special functions. And ofcourse make a nice PCB for it so it can be used neat and tidy on a VIC-20. With a small manual completing everything (although lot's of that manual was copied from the original SAM manual, which makes sense as this is in fact a reincarnation of SAM). The manual explaining the singing mode and device implements a demo of the famous song "Bicycle build for two" also known as "Daisy". Now in order to improve the usabillity of SAM I added the option of using a dictionary. This way SAM can be instructed to pronounce word differently, allowing for improved speech, alternative languages or censoring of "unwanted" words.

For more information, visit my website: https://janderogee.com

