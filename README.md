# Tinyphone
Minimalist Softphone based on PJSIP with API Control

[![AppVeyor](https://img.shields.io/appveyor/ci/kingster/tinyphone)](https://ci.appveyor.com/project/kingster/tinyphone/) 

## Getting Started

Checkout the repo and follow the building process to compile your own, or just download the latest version from the downloads section.

## Downloads

You can download the latest msi installer or executable file from [here](https://ci.appveyor.com/project/kingster/tinyphone/build/artifacts) or from the [releases section](https://github.com/voiceip/tinyphone/releases).


## APIs 

The softphone exposes the following resources on port `6060`. 

| Resource                    | Method | Description                       |
|:----------------------------|:-------|:----------------------------------|
| `/`      | GET | Returns `hi` and the app `version` |
| `/login`| POST | Account login with the provided details `AccountConfig` | 
| `/logout`| POST | Logout of all accounts |
| `/accounts`| GET | Returns list of registed accounts |
| `/accounts/{account_name}/logout`| GET | Logout of account with provided `account_name` |
| `/dial`| POST | Dial a call with provided `sip-uri` | 
| `/calls`| GET | Returns list of active calls |
| `/calls/{call_id}/dtmf/{digits}`| POST | Send dtmf digits `digits` to call with specified `call_id` | 
| `/calls/{call_id}/hold`| PUT | Hold call with specified `call_id` | 
| `/calls/{call_id}/hold`| DELETE | UnHold call with specified `call_id` | 
| `/calls/{call_id}/hangup`| POST | hangup call with specified `call_id` | 
| `/hangup_all`| POST | Hangup all calls | 
| `/exit`| POST | Exit the application | 

You can view the postman collection of the APIs [here](https://documenter.getpostman.com/view/111463/TVYC9zbp)


### Request Payloads

#### Account Config
```json
{
  "username": "string" ,
  "password" : "string",
  "domain" : "string",
  "proxy" : "optional-string*"
}
```
`* optional fields : Fields should exist only with valid value.`



## Issues & Contribution

Found a issue and have a solution? Go ahead and create a pull request :) Incase you don't have a immediate solution but have an replicable issue please create an Issue with the details so that we can have a look at it.


### Pre Requsite

You must have boost libraries installed on your system. Incase you don't have you can either download the [sources](https://www.boost.org/users/download/) and build it, or install a precompiled binaries from [here](https://sourceforge.net/projects/boost/files/boost-binaries/).


### Manual Compiling

```bash
git clone https://github.com/voiceip/tinyphone.git
cd tinyphone
git submodule update --init --recursive
#build dependencies follow appveyor.yml#build-step
#open lib/pjproject/pjproject-vs14.sln in visual studio 2015 and compile it
#open tinyphone/tinyphone.sln in visual studio 2015 and compile it (you will need to compile the dependencies in lib/* folder)
run tinyphone/Debug/tinyphone.exe
```
