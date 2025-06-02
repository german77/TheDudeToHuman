# TheDudeToHuman
This is a Mikrotik The Dude database exporter. Pulls the database from your Mikrotik device loads the contents and saves the contents into a json file.

## Why
There's no support from Mikrotik to access this data easily. Either by API, CLI or the dude itself. The most useful tool Mikrotik provides is to export to csv which omits a lot data. More importantly the object index. Which is used to easily identify a device from the database.

The Dude database uses SQLite. But all the objects are stored in a proprietary binary blob rendering useless for querying any kind of info out of the database. This program parses every single binary blob into a well defined structure with field names.

## Security concerns
The database stores IP address, usernames and passwords in plain text. Please keep all your backups and data stored by this program with care.

# Usage

Provide either database file or mikrotik device. Specify the output path and wait until the program it's done. This may take several minutes depending on the database size.

```bash
Usage: the_dude_to_human.exe [options] <filename>
-f, --file                                 Load the specified database file
-o, --out                                  Save json database file
-c, --credentials                          Save credentials in plain text
-m, --mikrotik=user:password@address:port  Connect to the specified mikrotik device
-h, --help                                 Display this help and exit
-v, --version                              Print tool version

Address format examples:
    user@192.168.1.1                       IP address
    user@domain.name                       Domain name
    user@192.168.1.1:1234                  User defined port
    user:password@192.168.1.1              User defined password
    user:@192.168.1.1                      Hidden user defined password
```
## Example
```bash
./the_dude_to_human -f dude.db -o dude.json
```

Expected output

```json
{
"serverConfig": [{"objectId":10000, "name":"Server Configuration", ...}],
"tool": [{"objectId":10004, "name":"Ping", "builtin":true, ...}],
"file": [{"objectId":20001, "name":"Vera", "fileName":"Vera.ttf"} ...],
"notes": [{"objectId":30002, "name":"This service is usef...", ...}],
"map": [{"objectId":50021, "name":"Main", "useStaticColor":false, ...}],
"probe":  [{"objectId":40068, "name":"ping", "snmpOid":[], ...}],
"deviceType":  [{"objectId":60023, "name":"ISP", "ignoredServices":[10060], ...}],
"device": [{"objectId":70035, "name":"ISP provider", "dnsNames":["domain"], ...}],
"network": [{"objectId":80131, "name":"", "subnets":[80132,80231], ...}],
"service": [{"objectId":90952, "name":"Service", "notifyIds":[90625,10022], ...}],
"notification": [{"objectId":75001, "name":"popup", "statusList":[69001,25001], ...}],
"link": [{"objectId":34021, "name":"Link", "history":false, "masteringType":0, ...}],
"linkType": [{"objectId":10285, "name":"gigabit ethernet", "thickness":6, ...}],
"dataSource": [{"objectId":35121, "name":"ether3 @ ISP provider rx", ...}],
"objectList": [{"objectId":61023, "name":"Throughput", "ordered":true, ...}],
"deviceGroup": [{"objectId":92030, "name":"Providers", "deviceIds":[]}],
"function": [{"objectId":42014, "name":"cpu_usage", "argumentDescriptors":[], ...}],
"snmpProfile": [{"objectId":10001, "name":"v1-public", "version":0, ...}],
"panel": [{"objectId":76032, "name":"Downloads", "locked":false, ...}],
"sysLogRule":[{"objectId":50021, "name":"Syslog Rule", "regexpNot":false, ...}],
"networkMapElement": [{"objectId":69058, "name":"[Device.Name]", ...}],
"chartLine": [{"objectId":13690, "name":"Chart Line", "sourceId":316924, ...}],
"panelElement": [{"objectId":28844, "name":"", "split":false, ...}]
}
```

# Development

Make sure that the submodules are initialized.

```bash
git submodule update --init --recursive
```

Generate project structure with CMake with default settings and compile the project.

```bash
cmake -S . -B build
cmake --build build --target the_dude_to_human
```

## Future Goals
* Export the contents to a remote database
* Export into other formats sql, sqlite, csv, etc.
* Repair database corruption with minimal data loss
* Clean database. Reduce size by removing unused objects and polling data.
* Compact database. Overtime objects id get too large leading to integer overflows. This should reorganize settings to keep the database in good shape.
