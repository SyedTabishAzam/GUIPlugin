======================================================================
STAGE Sample
Monte Carlo Analysis
======================================================================

Purpose
-------
The purpose of this sample is to show how Monte Carlo type of
analysis can be done with STAGE.  Monte Carlo analysis requires 
running many tests successively in order to generate and collect 
data necessary for analysis and doing so without human intervention.
It also requires the capability of modifying some parameters (like the 
seed of the random number generator) for each test.  This sample does 
it by using STAGE in batch mode meaning that an external application is 
used to control the download and the execution of the scenario used 
for each test.

This sample provides two ways of configuring and executing batched
simulation runs: a script-based command line application and a GUI application.


Components
----------
The main components of this sample are:

    1. Source code,
    2. Command file,
    3. Windows batch files,
    4. STAGE SM data.

The provided source code consists in four projects:

    * SimBatchPlugin:
        A STAGE simulation plugin which receives simulation run parameters
        and monitors the stop conditions.

    * StageBatchLib:
        A Dynamic-Linked Library (DLL) that provides the common logic between
        the command line and GUI client applications.
        
    * StageBatchCmd:
        A command line client which executes simulation runs described in
        batch command files.

    * StageBatchGui:
        A graphical user interface client that allows creating, configuring
        and executing simulation runs.

A sample command file to be used with the command-line client,
"stage.commands", is included. This file also includes documentation on the
command file syntax.

Windows batch files are included to start the command line
and gui clients along with the necessary applications (windows only).

The STAGE SM data component is also broken in 2 parts; a STAGE SM
database named "batch" and a STAGE SM configuration file named
"config_mc.cfg" set up specifically for Monte Carlo analysis.
The differences with respect to the default configuration file
consist of the use of the Inline Terrain Service Type for the
simulation and using the Synchronize Results option for the Terrain
Server.  Also options to start automatically the simulation and the 
terrain server have been turned off.

S:\Presagis\CDB\CDB_BOOT
S:\Presagis\CDB\CDB_BOOT
Installation
------------
To build the sample:

On Windows, with Visual Studio(r):
    Open the solution StageBatch_vs2010.sln 
    (for VS2010) or StageBatch_vs2013.sln (for VS2013),
    and compile it to generate the user plugin.

On Linux:
    Use 'Make -f Makefile.batch' to build the command-line application.
    Use 'Make -f Makefile' to build the SIM user plugin.


**************
SPECIAL NOTE:
**************

    Once the SIM user plugin is built then the file named 
    "%stage_dir%\userplugins\sim\SimBatchPlugin.dll" is created.
    Make sure you remove that file once you are done with this
    sample in order to get back the normal functionality of the
    simulation process.


Running the Sample
------------------

By default, the sample is configured to run scenarios in CDB mode.
Before running the sample, the first step is to specify the CDB path
for the provided configuration file: config_mc.cfg. To do this, you
could follow the steps:
  - start stageSM,
  - from the Tools menu, select Configuration Manager...,
  - load configuration file: config_mc.cfg, provided in this sample,
  - within the Global category, specify the proper CDB path,
  - save the loaded configuration file.     
N33:23:26.354
W117:34:18.889
The second step is to start the CDB servers, use the following command:
startApp -C %STAGE_DIR%/samples/monte_carlo/config_mc.cfg

To start the batch application on windows, execute the
"start_batch_cmd" batch file in the sample directory. The script will
start the batch command line application and then start the SIM in 
standalone mode.

To start the batch application on Linux, you have to start each process;
the command line application "StageBatchCmd" and the simulation process 
"stage.sim". Both processes must be started from the sample directory
(where this readme is located). Also make sure the simulation process
is started in standalone mode, for instance:
stage.sim -S -C $STAGE_DIR/samples/monte_carlo/config_mc.cfg

Alternatively, on Windows, you can use the provided GUI application
to easily define your own tests. To do so, run the "start_batch_gui"
batch file from the sample directory. An interface should appear,
allowing you to create new tests, customize their parameters and
execute them.

Interpreting the output
-----------------------

Once the tests complete, you should notice output files of the form
"out_data.x.y" appear in the sample directory. The first number
corresponds to the test index and the second corresponds to its
iteration index, if the test has multiple iterations.

The data recorded in output files consists of:
    - the simulation time,
    - the entity name,
    - the entity damage level,
    - the entity speed,
    - the entity altitude,
    - the ground elevation at the entity position,
    - the entity heading.

The information is collected for each entity in the scenario.
You can modify the SIM plugin to log different values as needed.

Customizing the Tests
---------------------

Once you have succeeded running the provided tests, you
can create your own runs tailored to your needs. There are two
ways of doing this: using a commands file or using the GUI application.

The "stage.commands" file defines the tests executed by the
the command line application. This file includes comments describing
the commands you can use to create your own customized tests.

The provided "stage.commands" file contains 4 tests.  Without
performing an in-depth analysis, here are some interesting points
to notice:
    - different scenarios are used for the first 2 tests and
      thus the list of entities in the corresponding output files
      is different.
    - the first and third tests use the same scenario but the
      altitude and speed have been modified in the third one 
      through test definition commands.
    - the first and fourth tests are identical and shows that the
      same results are obtained.
    
Note that the SIM process executes in asynchronous mode
(as fast as possible). Therefore, if you indicate that the test should
stop after 10 simulation seconds, it might complete in only 2 seconds
of real clock time, for example.

Scenarios can be restored from snapshot files (.snb) instead
of being opened. This allows to restore the state of a scenario
that was previously saved during its execution. However, there
is a known limitation with snapshot restores: the SIM process
has to be launched with the database name explicitly specified,
using the -I command line option. For example:

stagesim -stand-alone -C %STAGE_DIR%\samples\monte_carlo\config_mc.cfg
    -I sample.xml
    
Displaying a Running Test (Windows Only)
----------------------------------------
   
As the SIM executes in stand-alone mode, without a STAGE Scenario
Manager instance, you will not see the scenario executing by default.
In order to see the scenario being executed, follow these steps:

    1. Launch a STAGE Scenario Manager instance in slave mode using
       the following command line:
       stagesm -S
       
    2. In the STAGE Scenario Manager, open the scenario file
       that is being executed by the simulation.
       
    3. Press the start simulation button, note that this will not
       actually start new a simulation process, but rather connect
       to the existing standalone simulation process and provide
       a display of the state of its scenario.

How it works
------------

The command line application loads and parses the "stage.commands"
file and will send commands to the SIM to control the execution
of each test.  Some test definition commands will be transferred 
directly to the SIM through a temporary script file named 
"stage.sim_cmd".  This is a temporary file since the external 
application will generate that file prior to the execution of each 
test.  Refer to the beginning of the "stage.commands" file for 
a list of commands available to define a test. The GUI client also
generates a temporary script, but does not read the "stage.commands"
file.

The SIM user plugin will then load and parse that temporary file
and execute the commands. It is also the responsibility of this
plugin to collect all data necessary for post analysis. Therefore,
it is the SIM plugin that generates the output file for each test
in the form "out_data.x.y".

Design of the Sample's Code
---------------------------
The main classes in this sample's code are as follow:

SimulationRun: Stores all parameters needed to describe a simulation
test, including scenario and database name, start and end conditions.
Unrecognized parameters parsed from the commands file by the
command-line client are stored in the SimParams collection of strings
and directly transferred to the sim when executed.

SimulationBatcher: Executes tests defined by SimulationRun objects.
As the execution of a test is a blocking operation, provides means
of listening to the status of the execution and of cancelling the
execution from another thread.

Sim: Encapsulates communication with the SIM process including sending
messages to load, start and stop scenarios and receiving messages to
track the status of the SIM.

To Modify the Sample
--------------------
If you only want to add and delete tests or change the names 
and the duration of scenarios to be run, you only need to 
modify the configuration file "stage.commands".

Note that you can also start the command line application with a 
file name in parameter to specify an alternate name for the 
configuration file.  This allows you to create your own file
instead of modifying the "stage.commands" file.

But if you need to specify new commands to be handled by
the SIM you will need to add code to the source files and then 
recompile a new user SIM plugin. This is done the following way:
    - in the file "um_batch.c", define a unique keyword representing 
      your new command in the "tab_config_args" table,
    - then add a new "case" for your new keyword to the "switch" 
      statement in the "parse_config_file()" function with the 
      corresponding action to be executed by the SIM,
    - recompile the SIM user plugin.
You are now ready to modify the configuration file ("stage.commands"
by default) to use your new keyword as test definition command.

To add new commands to be configured through the GUI client,
the following steps should be observed:
    - Add fields to the SimulationRun class to represent your
      new per-test data. Alternatively, you can simply use the
      SimParams collection of strings as its items will get
      transferred to the SIM plugin.
    - Modify the GUI dialog to add your new controls.
    - Modify StageBatchGuiDlg::vUpdateControlsFromRun and
      vUpdateRunFromControls to properly map GUI values to the
      SimulationRun object.
    - If you chose to add a field to the SimulationRun class,
      modify SimulationBatcher::WriteCommandsFile to write the
      field to the SIM commands file.
    - Modify the SIM plugin as described above to parse and handle
      your new command.

Contact
-------
For any questions regarding the sample please contact STAGE support at

  stage@presagis.com

or visit our web site at

  http://www.presagis.com/support/

======================================================================

AVIS IMPORTANT: ce logiciel, incluant le code source, contient de l’information
confidentielle et/ou propriété de Presagis Canada inc. et/ou ses filiales et
compagnies affiliées (« Presagis ») et est protégé par les lois sur le droit
d’auteur. L’utilisation de ce logiciel est sujet aux termes d’une licence de
Presagis.
Malgré toute disposition contraire, l'utilisateur titulaire 
d'une licence peut utiliser les exemples de code pour générer de nouvelles 
applications.
© Presagis Canada Inc., 1995-2015. Tous droits réservés.

IMPORTANT NOTICE:  This software program (“Software”) and/or  the Source
code is a confidential and/or proprietary product of Presagis Canada Inc.
and/or its subsidiaries and affiliated companies (“Presagis”) and is
protected by copyright laws.  The use of the Software is subject to the
terms of the Presagis Software License Agreement.
Notwithstanding anything to the contrary, the 
licensed end-user is allowed to use the sample source provided with the 
Software to generate new applications.
© Presagis Canada Inc., 1995-2015. All Rights Reserved.

STAGE is a trademark of Presagis Canada, Inc.

Microsoft and Visual Studio are either trademarks or registered 
trademarks of Microsoft Corporation in the United States and/or other 
countries.