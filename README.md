# Just an impl of Huffman compression
A very simple program just figuring out how to put together the Huffman compression algorithm. Simply download zip or clone and run `make` in root file. Should give you a `huff` executable. 

**To compress**
```bash
./huff -c <infile> <outfile>
```
**To decompress**
```bash
./huff -d <infile> <outfile>
```

Want to change the tree traversal bit generator into a lookup table. Right now compression is a little slow at about 20 seconds per 50Mb. Decompression speed is reasonable though. I would like to add would be `zip` like directory structures within the compressed file. TBD if I get around to doing that. I should also add an implicit extension that the program appends to a compressed file. 

Still need to clean up the code a bit.
