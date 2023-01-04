# Pictures Database Manager

## Description


## Preview
![pictDBM_server](https://user-images.githubusercontent.com/9269271/210625164-04890801-e3f2-4515-b4fe-b74d411e29ca.png)


## Commands available
```java
./pictDBM [COMMAND] [ARGUMENTS]
```
* <code>**help**</code>
<i>displays this help.</i>

* <code>**list** &lt;dbfilename&gt;</code>
<i>list pictDB content.</i>

* <code>**create** &lt;dbfilename&gt; [options]</code>
	<i>create a new pictDB</i>.<br>
	
		options are: 
			-max_files <MAX_FILES> : maximum number of files.
			-thumb_res <X_RES> <Y_RES> : resolution for thumbnail images.
			-small_res <X_RES> <Y_RES> : resolution for small images.


* <code>**read** &lt;dbfilename&gt; &lt;pictID&gt; [original|orig|thumbnail|thumb|small]</code><br>
	<i>read an image from the pictDB and save it to a file.<br>
	default resolution is "original".</i>

* <code>**insert** &lt;dbfilename&gt; &lt;pictID&gt; &lt;filename&gt;</code><br>
	<i>insert a new image in the pictDB.</i>

* <code>**delete** &lt;dbfilename&gt; &lt;pictID&gt;</code><br>
<i>delete picture pictID from pictDB.</i>

* <code>**gc** &lt;dbfilename&gt; &lt;tmp dbfilename&gt;</code><br>
<i>performs garbage collecting on pictDB. Requires a temporary filename for copying the pictDB.</i>

##Makefile commands

* `clean-all` Clear all objects files and executables generated by a call to `make`
* `server` Launch the server, reachable on your web browser at `localhost:8000` (default value)
* `style` Apply `astyle` on the whole project's `.c` and `.h` files
