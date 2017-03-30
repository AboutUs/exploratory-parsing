Exploratory Parsing
===================

Exploratory parsing is a way to discover structure within semi-structured data.

Installation
------------

Build and install exploring version of peg/leg.

	git clone git://github.com/AboutUs/pegleg.git
	cd pegleg
	make
	sudo make install

Install dependencies.

	gem install sinatra
	gem install haml
	gem install diffy

Download sample data.

	sh scripts/download_world_factbook.sh

Launch the server.

	rackup

Alternative Installation (Docker)
---------------------------------

	docker-compose run --rm bootstrap
	docker-compose up -d web

Then point your web browser at:  http://your-docker-host:8080/

Usage
-----

Enter the following parser. Choose Factbook. Press Run.

	char = letter | number | other-char
	letter = << [a-zA-Z] >>
	number = << [0-9] >>
	other-char = << . >>

A new Run appears. Refresh the page to see it advance.

Click the Run number to see output. Click on counts to see sample matches.

License
-------

Licensed under the MIT License.
