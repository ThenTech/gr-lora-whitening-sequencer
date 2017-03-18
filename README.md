# gr-lora-whitening-sequencer
Average whitening sequence bits for better decoding accuracy.

````
Usage: lora-whitening-sequencer [-h|-H] [-sf|-SF <uint>] [-hdr]
                                [-i|-in <file>] [-o|-out <file>]

  LoRa whitening sequencer expects a csv file with bytes in 0x.. format
  and will compare the bits of each byte on each row and write the highest
  occuring '1's to the specified file.

  -h,   -H,   --help             Show this help message
  -sf,  -SF   <uint>             Set the spreading factor
  -hdr,       --header           Input has header bits
  -i,   -in,  --input  <file>    Specify input csv file with data
  -o,   -out, --output <file>    Specify output file for appending sequence
  ````
  
### Example:
  
  * Input
````
input.csv:
    0x11,0x22,0xC3,0x44
    0x12,0x22,0x33,0x4D
    0x11,0x28,0x33,0x44
    0x11,0xa2,0x33,0x44
    0x1f,0x22,0x30,0x04
    
./lora-whitening-sequencer -sf 7 -in ./input.csv
````
  * Output
````
const uint8_t prng_payload_sf7[] = {
    0x11,0x22,0x33,0x44
};
````
