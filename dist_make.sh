#!/bin/sh
./dist_clean.sh
mkdir ../uade-2.13q
cp -a * ../uade-2.13q
tar -cjf uade_2.13q.tar.bz2 ../uade-2.13q
rm -rf ../uade-2.13q
sha1sum uade_2.13q.tar.bz2 > uade_2.13q.tar.bz2.sha1
mv uade_2.13q.tar.bz2 ..
mv uade_2.13q.tar.bz2.sha1 ..
