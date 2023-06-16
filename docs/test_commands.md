hacer que nc mande de a 1 byte
```
stty -icanon && nc -C <host> <port>
```

Para crear archivos grandes
```
dd if=/dev/urandom bs=4096 count=25600 of=<output-file>
```
bs: block size (4k en el ejemplo)
count: block count

```
base64 <file> > <output-file>
```

para hacer list
```
curl pop3://<user>:<password>@<host>:<port>/
```

para hacer retr de uno especifico
```
curl pop3://<user>:<password>@<host>:<port>/num > output
```

para comparar podemos usar
```
diff -u <file-1> <file-2>
```
o
```
md5sum <file-1>
md5sum <file-2>
```
