----------------------------------------------------------
* Contact Info *

Email: kobe@pku.edu.cn
Peking University, Beijing, China.


----------------------------------------------------------
* Description *

This is our submission to the 2014 mirex Audio Fingerprinting task. The algorithm consists of two separated parts, builder and matcher. The builder extracts audio fingerprints from mp3 files and write them to file in the disk. The matcher loads these descriptor and serve the queries.


----------------------------------------------------------
* Platform and Requirements *

The program runs under Linux platform, but it requires two dynamic link libraries, mpg123 and sndfile. So you need to guarantee these two packages have been installed. The commands to install these packages are:
sudo apt-get install libmpg123-dev
sudo apt-get install libsndfile1-dev


----------------------------------------------------------
* Use *

For builder: ./builder %fileList4db% %dir4db%
The %dir4db% can be not existed. The program will create it by itself.

For matcher: ./matcher %fileList4query% %dir4db% %resultFile%
