## <b>Introduction</b> ##


This repository is a fork of the HiFi-SoRaP project https://sourceforge.net/projects/hifi-sorap/.
The original project uses OpenGL to interact with the GPU. This fork integrates the Vulkan graphics API  as a way to use compute shaders for faster computing of the SRP.

Developed for the Math and Computer Science degrees final thesis by me. 

To install the project, the installation instructions below for the correspondent platform should be followed. Note that the OpenGL libraries are still needed to compile because some definitions from OpenGL still remain in the project. This includes the `glext.dll` noted below.

After installing Qt and the compiler, the Vulkan SDK should be downloaded from https://vulkan.lunarg.com/sdk/home and installed following https://vulkan.lunarg.com/doc/sdk/1.3.296.0/windows/getting_started.html.


The README of the original project developed by Leandro Zardaín follows, with some modifications:

The  aim  of  this  project  is  to  develop  an  efficient  way  of  computing  the solar  radiation  pressure
acceleration  that  acts  on  a  spacecraft.

This acceleration is produced by the impact of the photons emitted by the Sun onto a spacecraft's surface.
This collision generates an acceleration which affects its motion and has a relevant effect on a long-term propagation.

This proposal studies which models can be used to approximate the computation of this acceleration.
Some of the models are more efficient and some others are more accurate.  The objective was to find a model that is
both efficient and accurate.  In order to do so, some of the most accurate models were implemented using the GPU to
parallelize part of the computation. The chosen model that fulfils both conditionsis based on the Raytrace technique.
It considers secondary rays (bounces) in order to obtain more precision when computing the acceleration.

This project  allows the  user  to  compute  and compare  between  different  approximations  of  the  SRP acceleration, 
and decide which approximation should be used.
It also includes the possibility  of  visualizing  the  spacecraft  model  and  the  accelerations  for  a  better
understanding of them.

It was created by me (Leandro Zardaín Rodríguez, leandrozardain@gmail.com) under the guidance of the professors Anna Puig (annapuig@ub.edu) & Ariadna Farrés (ariadna.farres@gmail.com) for my Master degree thesis on 2019. We have made an article about this where you can get more information about this project:
<a href="https://www.researchgate.net/publication/349295985_High-fidelity_Modeling_and_Visualizing_of_Solar_Radiation_Pressure_A_Framework_for_High-fidelity_Analysis"><b>Farres, Ariadna & Puig, Anna & Zardaín, Leandro. (2020). High-fidelity Modeling and Visualizing of Solar Radiation Pressure: A Framework for High-fidelity Analysis. </b> </a>

<b>Project software architecture</b>

The solution that is proposed in this thesis consists of a C++ application,  using the Qt framework.  It was coded in C++
for the CPU part and GLSL (OpenGLShading Language) for the GPU part.  It includes the libraries of Eigen and glm, so the
user does not need to install them on their computer.

## <b>Installation guide</b> ##

It is required to have the 3.3 version of OpenGL (and 3.30 of GLSL).
We have used the version 9.3.0 of gcc compiler.
In order to compile this project it is recommended to install Qt 5.14.2 (Qt creator 4.12.4). It is a framework that allows you
to code and compile easily in C++ and GLSL for the shaders (GPU).

Operative Systems recommended: Linux, Mac. (It can also run on Windows)

<details>
<summary>Installation on Linux</summary>
	
1. Install Qt:
<br/>

    * Qt installer can be downloaded from <a href="https://www.qt.io/download-qt-installer?utm_referrer=https%3A%2F%2Fwww.qt.io%2Fdownload-open-source%3Futm_referrer%3Dhttps%253A%252F%252Fwww.qt.io%252Fdownload">here</a>.

<br/>

    * Open a terminal where the file was downloaded and run: "chmod +x 'downloaded_file_name'". Then, run: "./'downloaded_file_name'". 

<br/>

    * Check the things in the Qt installer as here:

<br/>

<a href="https://postimg.cc/Z05jmMWx" target="_blank"><img src="https://i.postimg.cc/fWxPc4Zp/qt-steps.png" alt="qt-steps"/></a><br/><br/>
<br/>

    * Try to open QtCreator and load the project. 
	If QtCreator doesn't start, run: "sudo apt-get install --reinstall qtcreator".

<br/>
    	When opening the project for the fisrt time, you may be asked to choose a kit (the compiler for the project). In our case, we have used the GCC one: 
<a href="https://postimg.cc/JskPgHsw" target="_blank"><img src="https://i.postimg.cc/13W28pPz/qt-steps2.png" alt="qt-steps2"/></a><br/><br/>

<br/>
<br/>
2. Install OpenGL:

    * Run: "sudo apt-get install libgl-dev".
	
<br/><br/>
</details>

<details>
<summary>Installation on Mac</summary>

1. Install Qt

<br/>
    * Qt installer can be downloaded from <a href="https://www.qt.io/download-qt-installer?utm_referrer=https%3A%2F%2Fwww.qt.io%2Fdownload-open-source%3Futm_referrer%3Dhttps%253A%252F%252Fwww.qt.io%252Fdownload">here</a>.

<br/>
    * Check the things in the Qt installer as here:

<br/>
<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/SNhw3SDm/qt-steps-mac.png" alt="qt-steps-mac"/></a><br/><br/>	
    <a href="https://postimg.cc/svtJpfwT" target="_blank"><img src="https://i.postimg.cc/jSjkYLNY/qt-steps-mac2.png" alt="qt-steps-mac2"/></a><br/><br/>	
<br/>
    Inside the Qt version choosen (for example, "Qt 5.14.2"), enable also the macOS toggle:<br/>
<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/wMhnMZTK/qt-steps-mac3.png" alt="qt-steps-mac3"/></a><br/><br/>	
<br/>
    When opening the project for the first time, you may be asked to choose a kit (the compiler for the project). For example, you can choose the clang one:
    <a href='https://postimages.org/' target='_blank'><img src='https://i.postimg.cc/tggTP0fH/qt-step-3-5.png' border='0' alt='qt-step-3-5'/></a>

<br/>
    On Qt, in case you have a problem compiling the project with qmake:
	1. Select the tab 'Projects' in the left side tabs. It will take you to the 'Build Settings' page.
    <a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/pLV3Vsw7/qt-steps-mac4.png" alt="qt-steps-mac4"/></a><br/><br/>	
<br/>
	2. Add "INCLUDEPATH+=/opt/X11/include" in the qmake options:

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/vTRpQbyL/qt-steps-mac5.png" alt="qt-steps-mac5"/></a><br/><br/>
<br/>
<br/>
2. Install OpenGL:

    * Install last version of Xcode and XQuartz from the Mac AppStore.

<br/>
</details>

<details>
<summary>Installation on Windows</summary>

1. Install Visual Studio (this is needed for the C++ compiler):
    * You need to choose which version of Visual Studio you want, we recommend the 2022 Community version.
<br/>
<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/Xvphdp7k/visual-studio-1.png" alt="visual-studio-1"/></a><br/><br/>
<br/>
* Check the next things when installing Visual Studio:

* Edit: Choose also "MSVC v141 - VS 2017 C++ x64/x86 build tools (v14.16)", as MSVC2017 is the supported compiler for the Qt version that will be installed. MSVC2019 should be also compatible, but it was not tested.
	
<br/>
<a href="https://postimg.cc/8sX4zmWQ" target="_blank"><img src="https://i.postimg.cc/rFMPghwm/visual-studio-2.png" alt="visual-studio-2"/></a><br/><br/>
<a href="https://postimg.cc/BLqpXhMs" target="_blank"><img src="https://i.postimg.cc/zBhc26hL/visual-studio-3.png" alt="visual-studio-3"/></a><br/><br/>
    <a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/BbGwrp5m/visual-studio-4.png" alt="visual-studio-4"/></a><br/><br/>
<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/D0dYnGBn/visual-studio-5.png" alt="visual-studio-5"/></a><br/><br/>	
<br/>
<br/>
2. Install Qt

* Qt installer can be downloaded from <a href="https://www.qt.io/download-qt-installer?utm_referrer=https%3A%2F%2Fwww.qt.io%2Fdownload-open-source%3Futm_referrer%3Dhttps%253A%252F%252Fwww.qt.io%252Fdownload">here</a>.

<br/>
    * Check the things in the Qt installer as here:

<br/>
<a href="https://postimg.cc/hfJrzQbJ" target="_blank"><img src="https://i.postimg.cc/qMwZF8NG/qt-steps-windows.png" alt="qt-steps-windows"/></a><br/><br/>
	
<br/>
    * Try to open QtCreator and load the project. When opening the project for the fisrt time, you may be asked to choose a kit (the compiler for the project).
    In our case, we have used the MSCV one:
<a href="https://postimg.cc/tY7r3My4" target="_blank"><img src="https://i.postimg.cc/RVLjzkDK/qt-steps-windows2.png" alt="qt-steps-windows2"/></a><br/><br/>

The MSVC2017 32 bit compiler is needed. If Qt doesn't automatically detect this compiler, it can be manually setup, with Inizialization pointing to "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" and arguments as the image:

<a href="https://postimg.cc/F1y1ptth" target="_blank"><img src="https://i.postimg.cc/nV57HZ09/image.png" alt="compiler-setup"/></a>

<br/>
<br/>
3. Regarding OpenGL:
    * The library of OpenGL if already in the project. However, if it requires you the file "glext.dll" or the program crashes when running the project on Qt, this dll can be found in the RayTracingSRP folder. Put this dll file in the folder where there are the compiled objects of this project. This binary is 32 bits compiled, therefore the compiler needed is 32  bits.

<br/>
<br/>
</details>

<br/>
## <b>User guide: step by step</b> ##

Note: careful with decimal values (depending on your compiler you may need to change the ',' for '.' and viceversa in the files you want to load).

<details>
<summary>1. Load Spacecraft Model</summary>

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/3rpst4jL/Load-Object-2.png" alt="Load-Object-2"/></a><br/><br/>
<br/>
You need to load the spacecraft model (OBJ file) which is based on CAD model: it contains the list of vertices, faces 
and normals (optional). Also, you need a MTL file where is described the reflectivy properties of the surface of
the spacecraft. In addition, you can set its weight. In the resources/model directory there are examples of this files.
<br/>
<br/>
</details>

<details>
<summary>2. Choose a Method to compute SRP Acceleration</summary>

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/TPFs9fKM/Select-Method-2.png" alt="Select-Method-2"/></a><br/><br/>
<br/>
<br/>
You need to choose a method (the model you want to use to approximate the SRP force):

<br/>
<br/>
*Cannonball (CPU): considers the shape of the spacecraft to be a sphere. Ypu can set the area of the sphere (A) and the reflectivity
    property (Cr).

<br/>
<br/>
*NPlate (CPU): considers the shape of the spacecraft to be represented as a set of flat plates (you need to load a file that contains
    the number of plates, and then, for each plate, a new line with the area, specular reflectivity property, diffuse reflectivity
    property, and normal of the plate; you can see an example in the resources/model directory).

<br/>
<br/>
*RayTrace (CPU): for each cell of a grid defined by the number of cells (Nx x Ny) a ray is casted against the triangular mesh of the
    spacecraft. Then, it is computed the SRP force on the intersected triangle. The user can set the grid and secondary and diffuse
    rays.

<br/>
<br/>
*RayTrace (GPU): Similar to the CPU version, in this case the computation is done in the GPU. The user can set the secondary and diffuse rays.
    (Nx = Ny = 512 by default).

<br/>
<br/>
</details>

<details>
<summary>3. Choose an Action to perform</summary>

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/fLSGnkGR/Choose-Action-2.png" alt="Choose-Action-2"/></a><br/><br/>
<br/>
You need to choose what action you want to do:
	
<br/>
<br/>
* Visualize spacecraft: when the user press the start button in the Visualize Spacecraft tab, it will show a 3D viewer of the spacecraft with
    its 3 axes and the sunlight direction. Then, the user can set the initial rotation of the spacecraft by interacting with
    the three sliders. Each one of them corresponds to one of the local axes of the spacecraft. 

<br/>
<br/>
    For example, in the first slider, it appears "X" in red and this indicates that the red line in the 3D viewer correspond to the x axis. 	
    The user can also rotate the scene by pressing the right button of the mouse. It will not affect the computation of the SRP
    accelerations because it is modifying the orientation and position of the observer (camera) and nor the model neither the sunlight
    direction. And having pressed the left button, the user can zoom in and zoom out.
<br/>
<br/>

    Also, it allows the user to compute and visualize particular accelerations by rotating the spacecraft after having pressed the â€™Startâ€™
    button and consequently, interacted with the sliders.

<br/>
<br/>
    These accelerations that will appear in the 3D scene would have a different colour depending on their magnitudes. It was chosen to use
    the heat map colours to represent in blue the forces with lower magnitudes and in red the ones with higher magnitudes. Also, in this
    window it indicates which is the lowest and the highest magnitude among the accelerations that were computed.
<br/>
<br/>

* Visualize Graphics: it computes the SRP acceleration considering a set of pairs of azimuth and elevation angles. The user can select the
    azimuth and the elevation steps (they indicate how many points are used to discretize sample points from a sphere).

<br/>
<br/>
    if a GPU-based method was selected, another option would be added to this tab and it allows the user to visualize the results obtained
    from SRP accelerations while the computation of the global accelerations is being done.

<br/>
<br/>
    If the user pressed the "Start" button, these accelerations would be represented in a window with four 3D viewers showing each one of the
    components of the acceleration (x, y and z) and also its magnitude (see Fig. 17 and 18). In addition, the user can download the result
    as a txt file.

<br/>
<br/>
* Compare Graphics: this option allows the user to compare the result of two graphics that were previously generated. It is important to have this
tool of comparison because it lets the user to compute the difference between two already computed graphics. Also, it shows the mean
square error (MSE) and the maximum difference between the points on the charts.
<br/>
</details>

