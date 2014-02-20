#!/bin/bash

./main | awk '{printf("%.4f %s\n", NR*0.1, $0);}' | ~/modviz/modviz_cairo modviz.xml -
