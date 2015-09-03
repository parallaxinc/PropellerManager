#!/bin/bash

rm -rf html
doxygen

pushd html

for f in `find . -name _\* | sed -e 's/^\.\///'`
do
    g=`echo $f | sed -e's/^_//'`
    for i in *
    do
        sed -i $i -e 's/'$f'/'$g'/g'
    done
done

for f in `find . -name _\* | sed -e 's/^\.\///'`
do
    g=`echo $f | sed -e's/^_//'`
    mv $f $g
done

popd
