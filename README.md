# Project of Systems Programing - 2020/2021
## Description
This project was developed for the Systems Programming course, part of the Aerospace Engineering avionics master curriculum at Instituto Superior Técnico - Universidade de Lisboa. It consists of three different applications:
* A Local Server, that can contain several groups, each one with a key-value store table;
* A library and test application to connect and interact with the Local Server;
* An Authentication Server to generate passwords for groups and authenticate applications trying to interact with the tables associated with said groups.  

A full report of the project (in portuguese) can be found [here.](report.pdf)


## How to run
To compile on Linux, just do `make All`. This will generate 3 .out files, one for the Local Server, other for the Authentication Server and, finally, one for the test app. The test app requires no input arguments, while the authentication server requires the PORT it will run on and the local server needs the IP and PORT of the authentication server (separated by space, in this order).

## Authors:
* José Alberto Cavaleiro Henriques - ist189684
* Pedro Silva André - ist189707

