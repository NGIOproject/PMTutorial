#!/bin/bash

echo "------------------------------"
echo "KV1 Test"
./kv1 mypool
echo "------------------------------"
./kv1_answer mypool

echo "------------------------------"
echo "------------------------------"

echo "KV2 Test"
./kv2 mypool
echo "------------------------------"
./kv2_answer mypool

echo "------------------------------"
echo "------------------------------"

echo "KV3 Test"
./kv3 mypool
echo "------------------------------"
./kv3_answer mypool

echo "------------------------------"
echo "------------------------------"

echo "ARRAY1 Test"
./array1 mypool
echo "------------------------------"
./array1_answer mypool

echo "------------------------------"
echo "------------------------------"

echo "MKV1 Test"
./mkv1 mypool
echo "------------------------------"
./mkv1_answer mypool
