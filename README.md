# Tinyphone
Minimalist Softphone based on PJSIP with API Control

[![Tinyphone Build](https://github.com/voiceip/tinyphone/actions/workflows/main.yml/badge.svg)](https://github.com/voiceip/tinyphone/actions/workflows/main.yml)

## Getting Started

Checkout the repo and follow the building process to compile your own, or just download the latest version from the downloads section.

## Installation

You can download the latest msi installer from [here](https://ci.appveyor.com/project/kingster/tinyphone/build/artifacts) or from the [releases section](https://github.com/voiceip/tinyphone/releases).

*[Optional]* After Installing you can customise the tinyphone properties by downloading and placing the [config file](https://raw.githubusercontent.com/voiceip/tinyphone/HEAD/config.json) in the folder `C:\Program Files (x86)\Tinyphone\`.


## APIs 

The softphone exposes the following resources on port `6060`. 



<table>
<thead>
<tr>
<th>Resource</th>
<th>Method</th>
<th>Payload</th>
<th>Description</th>
</tr>
</thead>
<tbody>
<tr>
<td><code>/</code></td>
<td>GET</td>
<td></td>
<td>Returns <code>hi</code> and the app <code>version</code></td>
</tr>
<tr>
<td><code>/events</code></td>
<td>WS</td>
<td></td>
<td>WebSocket endpoint for realtime events</td>
</tr>
<tr>
<td><code>/login</code></td>
<td>POST</td>
<td>
<pre lang="json">
{
"username": "string" ,
"login": "optional-string**" ,
"password": "string",
"domain": "string",
"proxy": "optional-string**"
}
</pre>
</td>
<td>Account login with the provided details

</td>
</tr>
<tr>
<td><code>/logout</code></td>
<td>POST</td>
<td></td>
<td>Logout of all accounts</td>
</tr>
<tr>
<td><code>/accounts</code></td>
<td>GET</td>
<td></td>
<td>Returns list of registed accounts</td>
</tr>
<tr>
<td><code>/accounts/{account_name}/logout</code></td>
<td>GET</td>
<td></td>
<td>Logout of account with provided <code>account_name</code></td>
</tr>
<tr>
<td><code>/dial</code></td>
<td>POST</td>
<td>
<pre lang="json">
{
"uri": "sip-uri",
"account": "account_name**" 
}
</pre>  
</td>
<td>Dial a call with provided <code>sip-uri</code></td>
</tr>
<tr>
<td><code>/calls</code></td>
<td>GET</td>
<td></td>
<td>Returns list of active calls</td>
</tr>
<tr>
<td><code>/calls/{call_id}/answer</code></td>
<td>POST</td>
<td></td>
<td>answer call with specified <code>call_id</code></td>
</tr>
<tr>
<td><code>/calls/{call_id}/dtmf/{digits}</code></td>
<td>POST</td>
<td></td>
<td>Send dtmf digits <code>digits</code> to call with specified <code>call_id</code></td>
</tr>
<tr>
<td><code>/calls/{call_id}/hold</code></td>
<td>PUT</td>
<td></td>
<td>Hold call with specified <code>call_id</code></td>
</tr>
<tr>
<td><code>/calls/{call_id}/hold</code></td>
<td>DELETE</td>
<td></td>
<td>UnHold call with specified <code>call_id</code></td>
</tr>
<tr>
<td><code>/calls/{call_id}/conference</code></td>
<td>PUT</td>
<td></td>
<td>Create conference by merging other running calls with given <code>call_id</code></td>
</tr>
<tr>
<td><code>/calls/{call_id}/conference</code></td>
<td>DELETE</td>
<td></td>
<td>Break specified <code>call_id</code> out of conference</td>
</tr>
<tr>
<td><code>/calls/{call_id}/join/{call_to_join_id}</code></td>
<td>POST</td>
<td></td>
<td>Merges the current call (<code>call_id</code>) with another running call (<code>call_to_join_id</code>)</td>
</tr>
<tr>
<td><code>/calls/{call_id}/unjoin/{call_to_unjoin_id}</code></td>
<td>POST</td>
<td></td>
<td>Break specified <code>call_id</code> out of conference with <code>call_to_join_id</code></td>
</tr>
<tr>
<td><code>/calls/{call_id}/transfer</code></td>
<td>POST</td>
<td>
<pre lang="json">
{
"uri": "sip-uri",
}
</pre> 
</td>
<td>transfer <code>call_id</code> to specified <code>uri</code></td>
</tr>
<tr>
<td><code>/calls/{call_id}/attended-transfer/{dest_call_id}</code></td>
<td>POST</td>
<td></td>
<td>Initiate attended call transfer <br /><code>call_id</code>=The call id to be transfered <br /><code>dest_call_id</code>=The call id to be replaced</td>
</tr>
<tr>
<td><code>/calls/{call_id}/hangup</code></td>
<td>POST</td>
<td></td>
<td>hangup call with specified <code>call_id</code></td>
</tr>
<tr>
<td><code>/hangup_all</code></td>
<td>POST</td>
<td></td>
<td>Hangup all calls</td>
</tr>
<tr>
<td><code>/exit</code></td>
<td>POST</td>
<td></td>
<td>Exit the application</td>
</tr>
</tbody>

</table>

`** Optional Fields, should exist only with valid value`

You can view the postman collection of the APIs [here](https://documenter.getpostman.com/view/111463/TVYC9zbp)


## Building Locally

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

## Issues & Contribution

Found a issue and have a solution? Go ahead and create a pull request :) Incase you don't have a immediate solution but have an replicable issue please create an Issue with the details so that we can have a look at it.

