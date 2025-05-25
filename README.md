# TheDudeToHuman
This is a Mikrotik The Dude database exporter. Pulls the database from your mikrotik device loads the contents and saves the contents into a json file.

## Why
There's no support from Mikrotik to access this data easily. Either by API, CLI or the dude itself. The most useful tool mikrotik provides is to export to csv which omits a lot data. More importantly the object index. Which is used to easily identify a device from the database.

The Dude database uses SQLite. But all the objects are stored in a proprietary binary blob rendering useless for querying any kind of info out of the database. This program parses every single binary blob into a well defined structure with field names.

## Security concerns
The database stores IP address, usernames and passwords in plain text. Please keep all your backups and data stored by this program with care.

# Usage

Provide either database file or mikrotik device. Specify the output path and wait until the program it's done. This may take several minutes depending on the database size.

```bash
Usage: the_dude_to_human.exe [options] <filename>
-f, --file                                 Load the specified database file
-o, --out                                  Store database location
-m, --mikrotik=user:password@address:port  Connect to the specified mikrotik device
-h, --help                                 Display this help and exit
```

# Development

Make sure that the submodules are initialized.

```bash
git submodule update --init --recursive
```

Generate project structure with CMake with default settings and compile the project.

## Future Goals
* Export the contents to a remote database
* Export into other formats sql, sqlite, csv, etc.
* Repair database corruption with minimal data loss
* Clean database. Reduce size by removing unused objects and polling data.
* Compact database. Overtime objects id get too large leading to integer overflows. This should reorganize settings to keep the database in good shape.
