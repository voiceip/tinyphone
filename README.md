# tinyphone
Minimalist Softphone based on PJSIP with API Control

## Getting Started

Checkout the repo and follow the building process to compile your own, or just download the latest version from the releases page.

### Pre Requsite

You must have boost libraries installed on your system. Incase you don't have you can either download the [sources](https://www.boost.org/users/download/) and build it, or install a precompiled binaries from [here](https://sourceforge.net/projects/boost/files/boost-binaries/).


### Manual Compiling

```bash
git checkout https://github.com/voiceip/tinyphone.git
cd tinyphone
git submodule update --init --recursive
#open lib/pjproject/pjproject-vs14.sln in visual studio 2015 and compile it
#open tinyphone/tinyphone.sln in visual studio 2015 and compile it (you will need to compile the dependencies in lib/* folder)
run tinyphone/Debug/tinyphone.exe
```


## APIs 

The softphone exposes the following resources.

| Resource                    | Method | Description                       |
|:----------------------------|:-------|:----------------------------------|
| `/`      | GET | Returns `hello world` |
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


## Request Payloads

### Account Config
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
