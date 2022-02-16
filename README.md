## <b>Introduction</b> ##

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
[<b>Farres, Ariadna & Puig, Anna & Zardaín, Leandro. (2020). High-fidelity Modeling and Visualizing of Solar Radiation Pressure: A Framework for High-fidelity Analysis. </b>](https://www.researchgate.net/publication/349295985_High-fidelity_Modeling_and_Visualizing_of_Solar_Radiation_Pressure_A_Framework_for_High-fidelity_Analysis)

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

    * Open a terminal where the file was downloaded and run: "chmod +x <downloaded_file_name>". Then, run: "./<downloaded_file_name>". 

<br/>

    * Check the things in the Qt installer as here:

<br/>

    ![Qt-steps](https://user-images.githubusercontent.com/6904485/149152638-e967d5f9-7c89-495f-b0b2-62e7ebb9e137.PNG)

<br/>

    * Try to open QtCreator and load the project. 
	If QtCreator doesn't start, run: "sudo apt-get install --reinstall qtcreator".

<br/>
    	When opening the project for the fisrt time, you may be asked to choose a kit (the compiler for the project). In our case, we have used the GCC one: 
    	![imagen](https://user-images.githubusercontent.com/6904485/151379331-40c20dad-5cae-4305-890c-c8da1f459740.png)


<br/>
<br/>
2. Install OpenGL:

    * Run: "sudo apt-get install libgl-dev".
	
</details>

<details>
<summary>Installation on Mac</summary>

1. Install Qt

<br/>
    * Qt installer can be downloaded from <a href="https://www.qt.io/download-qt-installer?utm_referrer=https%3A%2F%2Fwww.qt.io%2Fdownload-open-source%3Futm_referrer%3Dhttps%253A%252F%252Fwww.qt.io%252Fdownload">here</a>.

<br/>
    * Check the things in the Qt installer as here:

<br/>
    ![dataVis](https://user-images.githubusercontent.com/6904485/151007196-a0004404-7bbc-4039-beff-802148c69f2d.png)
    ![Qt-steps](https://user-images.githubusercontent.com/6904485/149152638-e967d5f9-7c89-495f-b0b2-62e7ebb9e137.PNG)
	
<br/>
    Inside the Qt version choosen (for example, "Qt 5.14.2"), enable also the macOS toggle:
    ![macOs](https://user-images.githubusercontent.com/6904485/151008145-d86ac2fe-71c2-4737-98da-cc9d21e9c213.png)
	
<br/>
    When opening the project for the fisrt time, you may be asked to choose a kit (the compiler for the project). For example, you can choose the clang one:
    ![chooseKit](https://user-images.githubusercontent.com/6904485/151008592-0cd47cfe-5ca8-47e7-98c0-ade7fd59c11f.png)
	
<br/>
    On Qt, in case you have a problem compiling the project with qmake:
	1. Select the tab 'Projects' in the left side tabs. It will take you to the 'Build Settings' page.
		![imagen](https://user-images.githubusercontent.com/6904485/151010498-9ddac5ae-61bd-403d-8588-fe0805e59226.png)
	
<br/>
	2. Add "INCLUDEPATH+=/opt/X11/include" in the qmake options:

		![imagen](https://user-images.githubusercontent.com/6904485/151009640-527e7ff4-83bc-42e9-9594-19f3c0b5c945.png)

<br/>
<br/>
2. Install OpenGL:

    * Install last version of Xcode and XQuartz from the Mac AppStore.

</details>

<details>
<summary>Installation on Windows</summary>

1. Install Visual Studio (this is needed for the C++ compiler):
    * You need to choose which version of Visual Studio you want, we recommend the 2022 Community version.
<br/>

    ![VS_steps](https://user-images.githubusercontent.com/6904485/149152665-ffe51f07-ad75-4711-beac-c75b1060a6ab.PNG)

<br/>
    * Check the next things when installing Visual Studio:
	
<br/>
    ![VS_steps2](https://user-images.githubusercontent.com/6904485/149152672-6210fcf0-6909-4be9-8bd5-1d8b7adc7db1.PNG)
    ![VS_steps3](https://user-images.githubusercontent.com/6904485/149152680-9a990ba6-70cf-4f7a-b716-b91333e084b5.PNG)
    ![VS_steps4](https://user-images.githubusercontent.com/6904485/149152686-69a68e1b-f912-46ca-b4bc-20ac3dadeb91.PNG)
    ![VS_steps5](https://user-images.githubusercontent.com/6904485/149152695-248869c0-7d05-43d9-b361-ff01afec9caa.PNG)
	
<br/>
<br/>
2. Install Qt

    * Qt installer can be downloaded from <a href="https://www.qt.io/download-qt-installer?utm_referrer=https%3A%2F%2Fwww.qt.io%2Fdownload-open-source%3Futm_referrer%3Dhttps%253A%252F%252Fwww.qt.io%252Fdownload">here</a>.

<br/>
    * Check the things in the Qt installer as here:

<br/>
    ![Qt-steps](https://user-images.githubusercontent.com/6904485/149152638-e967d5f9-7c89-495f-b0b2-62e7ebb9e137.PNG)
	
<br/>
    * Try to open QtCreator and load the project. When opening the project for the fisrt time, you may be asked to choose a kit (the compiler for the project).
    In our case, we have used the MSCV one:
    	![imagen](https://user-images.githubusercontent.com/6904485/151386955-7840de61-179f-4bcc-a7a9-ff315fe838b1.png)

<br/>
<br/>
3. Regarding OpenGL:
    * The library of OpenGL if already in the project. However, if it requires you the file "glext.dll" or the program crashes when running the project on Qt, this dll can be found in the RayTracingSRP folder. Put this dll file in the folder where there are the compiled objects of this project.

</details>

## <b>User guide: step by step</b> ##

Note: careful with decimal values (depending on your compiler you may need to change the ',' for '.' and viceversa in the files you want to load).

<details>
<summary>1. Load Spacecraft Model</summary>

![UploadSpacecraft](https://user-images.githubusercontent.com/6904485/150852545-8f3d573e-9505-484b-be6f-1ef3afb178d3.PNG)

<br/>
You need to load the spacecraft model (OBJ file) which is based on CAD model: it contains the list of vertices, faces 
and normals (optional). Also, you need a MTL file where is described the reflectivy properties of the surface of
the spacecraft. In addition, you can set its weight. In the resources/model directory there are examples of this files.
</details>

<details>
<summary>2. Choose a Method to compute SRP Acceleration</summary>

![ChooseMethod](https://user-images.githubusercontent.com/6904485/150852542-7f6fe732-5e40-465f-84f1-0158ccdd62d4.PNG)

<br/>
You need to choose a method (the model you want to use to approximate the SRP force):

<br/>
* **Cannonball (CPU)**: considers the shape of the spacecraft to be a sphere. Ypu can set the area of the sphere (A) and the reflectivity
    property (Cr).

<br/>
* **NPlate (CPU)**: considers the shape of the spacecraft to be represented as a set of flat plates (you need to load a file that contains
    the number of plates, and then, for each plate, a new line with the area, specular reflectivity property, diffuse reflectivity
    property, and normal of the plate; you can see an example in the resources/model directory).

<br/>
* **RayTrace (CPU)**: for each cell of a grid defined by the number of cells (Nx x Ny) a ray is casted against the triangular mesh of the
    spacecraft. Then, it is computed the SRP force on the intersected triangle. The user can set the grid and secondary and diffuse
    rays.

<br/>
* **RayTrace (GPU)**: Similar to the CPU version, in this case the computation is done in the GPU. The user can set the secondary and diffuse rays.
    (Nx = Ny = 512 by default).

</details>

<details>
<summary>3. Choose an Action to perform</summary>

![ChooseAction](https://user-images.githubusercontent.com/6904485/150852539-9c57962f-dbea-4cf9-98f0-20e32776e414.PNG)

<br/>
You need to choose what action you want to do:
	
<br/>
* **Visualize spacecraft**: when the user press the start button in the Visualize Spacecraft tab, it will show a 3D viewer of the spacecraft with
    its 3 axes and the sunlight direction. Then, the user can set the initial rotation of the spacecraft by interacting with
    the three sliders. Each one of them corresponds to one of the local axes of the spacecraft. 

<br/>
    For example, in the first slider, it appears "X" in red and this indicates that the red line in the 3D viewer correspond to the x axis. 	
    The user can also rotate the scene by pressing the right button of the mouse. It will not affect the computation of the SRP
    accelerations because it is modifying the orientation and position of the observer (camera) and nor the model neither the sunlight
    direction. And having pressed the left button, the user can zoom in and zoom out.
<br/>

    Also, it allows the user to compute and visualize particular accelerations by rotating the spacecraft after having pressed the â€™Startâ€™
    button and consequently, interacted with the sliders.

    These accelerations that will appear in the 3D scene would have a different colour depending on their magnitudes. It was chosen to use
    the heat map colours to represent in blue the forces with lower magnitudes and in red the ones with higher magnitudes. Also, in this
    window it indicates which is the lowest and the highest magnitude among the accelerations that were computed.
<br/>

* **Visualize Graphics**: it computes the SRP acceleration considering a set of pairs of azimuth and elevation angles. The user can select the
    azimuth and the elevation steps (they indicate how many points are used to discretize sample points from a sphere).

<br/>
    if a GPU-based method was selected, another option would be added to this tab and it allows the user to visualize the results obtained
    from SRP accelerations while the computation of the global accelerations is being done.

<br/>
    If the user pressed the "Start" button, these accelerations would be represented in a window with four 3D viewers showing each one of the
    components of the acceleration (x, y and z) and also its magnitude (see Fig. 17 and 18). In addition, the user can download the result
    as a txt file.

<br/>
* **Compare Graphics**: this option allows the user to compare the result of two graphics that were previously generated. It is important to have this
tool of comparison because it lets the user to compute the difference between two already computed graphics. Also, it shows the mean
square error (MSE) and the maximum difference between the points on the charts.
</details>

