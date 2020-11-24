#!/bin/bash

# Install appdmg
# npm install -g appdmg


# Run program
rm -f tinyphone.dmg
appdmg release-spec.json tinyphone.dmg
