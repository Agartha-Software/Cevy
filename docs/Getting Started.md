# Getting Started
This section will help you get started on your Cevy journey as quilckly as possible. It will walk you through setting up your development environment an writing a simple Cevy app.

# Quick Start
If you want to drive in immediately and already have a working C++ setup, fell free to follow this "quick start" guid. Otherwis, move on to the next page.

## Try the Examples

1. Clone the [Cevy Repository][1]

```bash
git clone git@github.com:Arthur-Aillet/Cevy.git Cevy
```

2. Navigate to the new "Cevy" folder

```bash
cd Cevy
```

3. Switch to the version as you want or the latest version

``` Actually is not possible, we just have one version```

4. Try the exemple in the [R-Type project][2]

## Add Cevy as a Dependency

You can add in your CMakefile or in a specific file a FetchModule included in your CMakefile

```cmake
include(FetchContent)
FetchContent_Declare(
    cevy
    GIT_REPOSITORY https://github.com/Agartha-Sofware/Cevy
    GIT_TAG main (or another the tag version)
)
FetchContent_MakeAvailable(cevy)
```


# Setup

## C++ Setup
All Cevy app and engine is written in C++. This means that before we begin, we need to set up our C++ development environment.

You can check that you have installed C++ on our linux computer with this command:

```bash
gcc --version
```

### Installing C++

1. On **Debian**, **Ubuntu**, **Linux Mint**, **Deepin**

```bash
sudo apt-get install gcc
```

2. On **Arch Linux**, **Manjaro**, **ArkOS**, **EndeavourOS**

```bash
sudo pacman -S gcc
```

3. On **RedHat Enterprise Linux**, **Fedora**, **CentOS**, **SUSE**

```bash
sudo dnf install gcc
```


### Install OS dependencies

####    Linux

1. Ubuntu

```bash
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libglfw3-dev
```

2. Fedora

```bash
sudo dnf install alsa-lib-devel xorg-x11-server-devel libXrandr-devel libXi-devel mesa-libGL-devel mesa-libGLU-devel libXcursor-devel libXinerama-devel glfw-devel
```

3. Pacman

```bash
sudo pacman -S alsa-lib xorg-server libxrandr libxi mesa libglu libxcursor xorg-server-common libxinerama glfw-x11
```

####    Windows

- Run the [Visual Studio 2019 build tools installer][3]
- For easy setup, select the `Desktop development with C++` workload in the installer
- For a minimal setup, follow these steps:
  1. In the installer, navigate to `Individual components`
  2. Select the latest `MSVC` for your architecture and version of Windows
  3. Select the latest `Windows SDK` for your version of Windows
  4. Select the `C++ CMake tools` for Windows component
  5. Install the components


# Create a new Cevy
Now we are ready to set up a Cevy project! Cevy is just a normal C++ dependency. You can either add it to an existing C++ project or create a new one. For completeness we will assume you are starting from scratch.

## Create a new C++ executable project
First create a new folder for our project. For exemple, if you want to name your project "my_cevy_game", you can do so with the following command in your terminal:
```bash
mkdir my_cevy_game
cd my_cevy_game
```

In this folder, create a CMakeLists.txt file. This file is used by CMake to understand how to build your project. Here's a basic example for a C++ project:

```cmake
cmake_minimum_required(VERSION 3.10)

# set the project name
project(mon_projet_c++)

# add the executable
add_executable(mon_projet_c++ main.cpp)
```
In this example, we've defined a minimum version requirement for CMake, named our project and added an executable to be built from the main.cpp 1 file.

Create a main.cpp file in the same folder. This file will contain your program's main code. To start with, it can simply display "Hello, World!" as follows:


```cpp
#include <iostream>

int main() {
   std::cout << "Hello, World!";
   return 0;
}
```

## Add Cevy as a dependency

You can just modify your CMakeLists.txt at the root of your project.
```cmake
cmake_minimum_required(VERSION 3.10)

# set the project name
project(mon_projet_c++)

# add the executable
add_executable(mon_projet_c++ main.cpp)

include(FetchContent)
FetchContent_Declare(
    cevy
    GIT_REPOSITORY https://github.com/Arthur-Aillet/Cevy
    GIT_TAG main (or another the tag version)
)
FetchContent_MakeAvailable(cevy)
```
## Build Cevy
Now you can run your program with these line.

```bash
cmake -S . -B ./build
make --no-print-directory -C build
```

# Apps
Cevy programs are referred to as `App`s. The simplest Cevy app look like this:

```cpp
#include "App.hpp"

int main() {
    App app;
    app.run();
    return 0;
}
```

Nice and simple righ? Copy the code above into your `main.cpp` file and run it.
You will notice that nothing happens? This is because we haven't told our app to do anything yet! Apps are just empty shells capable of running our application logic. Let's add some logic to our App using Cevy ECS.


# ECS

All app logic in Cevy uses the Entity Component System paradigm, which is often shortened to ECS. ECS is a software pattern that involves breaking your program up into **Entities**, **Components**, and **Systems**. **Entities** are unique "things" that are assigned groups of **Components**, which are then processed using **Systems**.

For example, one entity might have a `Position` and `Velocity` component, whereas another entity might have a `Position` and `UI` component. Systems are logic that runs on a specific set of component types. You might have a `movement` system that runs on all entities with a `Position` and `Velocity` component.

The ECS pattern encourages clean, decoupled designs by forcing you to break up your app data and logic into its core components. It also helps make your code faster by optimizing memory access patterns and making parallelism easier.

## Your First System

Past the following function into your `main.cpp` file:
```cpp
void hello_world() {
    std::cout << "Hello World!" << std::endl;
}
```
This will be our fisrt system. The only remaining step is to add it to our `App`!

```cpp
#include "App.hpp"

int main() {
    App app;
    app.add_systems<core_stage::Update>(hello_world);
    app.run();
    return 0;
}
```
The `add_systems` function adds the system to your App's `Update Shedule`, but we'll cover that more later.
Now you can run your app again using. You should see `Hello World!` printed once in your terminal.

## Your First Components
Greeting the whole world is great, but what if we want to greet specific people? In ECS, you would generally model people as entities with a set of components that define them. Let's start simple with a `Person` component.

Create your class `Person`

```cpp
class Person {
    private:
        std::string _name;

    public:
        Person(std::string name) : _name(name) {}
        ~Person() {}

        std::string& getName() { return _name; }
};
```
We can then add people to our `World` using a "startup system". Startup systems are just like normal systems, but they run exactly once, before all other systems, right when our app starts. Let's use `Commands` to spawn some entities into our `World`:

```cpp
void add_people(Commands &cmd) {
    cmd.spawn(Person("Elaine Proctor"));
    cmd.spawn(Person("Renzo Hume"));
    cmd.spawn(Person("Zayna Nieves"));
}
```
Now register the startup system like this:

```cpp
#include "App.hpp"

int main() {
    App app;
    app.init_component<Person>();
    app.add_systems<core_stage::Startup>(add_people);
    app.add_systems<core_stage::Update>(hello_world);
    app.run();
    return 0;
}
```

## Yout First Query
We could run this now and the `add_people` system would run first, followed by `hello_world`. But our new people don't have anything to do yet! Let's make a system that properly greets the new citizens of our `World`

```cpp
void greet_people(Query<Name> persons) {
    for p : persons {
        std::cout << "hello " << p.getName() << std::endl;
    }
}
```
The parameters we pass into a "system function" define what data the system runs on. In this case, `greet_people` will run on all entities with the `Person` and `Name` component.

You can interpret the `Query` above as: "iterate over every `Name` component for entities that also have a `Person` component".

Now we just register the system in our `App`. Note that you can pass more than one system into an add_systems call by using a tuple!

```cpp
#include "App.hpp"

int main() {
    App app;
    app.init_component<Person>();
    app.add_systems<core_stage::Startup>(add_people);
    app.add_systems<core_stage::Update>(hello_world, greet_people);
    app.run();
    return 0;
}
```
Running our app will result in the following output:

```
hello world!
hello Elaina Proctor!
hello Renzo Hume!
hello Zayna Nieves!
Marvelous!
```

Quick Note: "hello world!" might show up in a different order than it does above. This is because systems run in parallel by default whenever possible.

# Plugins
One of Cevy's core principales is modularity. All Cevy engine features are implemented as plugins.This includes internal features like the renderer, but games themselves are also implemented as plugins! This empowers developers to pick and choose which features they want. Don't need a UI? Don't register the UiPlugin. Want to build a headless server? Don't register the RenderPlugin.

This also means you are free to replace any components you don't like. If you feel the need, you are welcome to build your own UiPlugin, but consider contributing it back to Cevy if you think it would be useful!

However, most developers don't need a custom experience and just want the "full engine" experience with no hassle. For this, Cevy provides a set of "default plugins".

## Cevy's Default Plugins
Let's make our app more interesting by adding the "default Cevy plugins".

```cpp
int main() {
    App app;
    app.add_plugins(DefaultPlugins());
    app.init_component<Person>();
    app.add_systems<core_stage::Startup>(add_people);
    app.add_systems<core_stage::Update>(hello_world, greet_people);
    app.run();
    return 0;
}
```

## Creating your first plugin
For better organization, let's move all of our "hello" logic to a plugin. To create a plugin we just need to implement the Plugin interface. Add the following code to your `main.cpp` file:

```cpp
#include "Plugin.hpp"

class HelloPlugin : public Plugin {
    public:
    void build(App &app) {
        // add things to your app here
    }
};
```
Then register the plugin in your App like this:

```cpp
int main() {
    App app;
    app.add_plugins(DefaultPlugins(), HelloPlugin());
    app.init_component<Person>();
    app.add_systems<core_stage::Startup>(add_people);
    app.add_systems<core_stage::Update>(hello_world, greet_people);
    app.run();
    return 0;
}
```
Note add_plugins can add any number of plugins (or plugin groups like DefaultPlugins) by passing in a tuple of them. Now all that's left is to move our systems into HelloPlugin, which is just a matter of cut and paste. The app variable in our plugin's build() function is the same builder type we use in our main() function:

```cpp
class HelloPlugin : public Plugin {
    public:
    void build(App &app) {
        app.init_component<Person>();
        app.add_systems<core_stage::Startup>(add_people);
        app.add_systems<core_stage::Update>(hello_world, greet_people);
    }
};

int main() {
    App app;
    app.add_plugins(DefaultPlugins(), HelloPlugin());
    app.run();
    return 0;
}
```
Try running the app again. It should do exactly what it did before. In the next section, we'll fix the "hello" spam using Resources.


# Resources
`Entities` and `Components` are great for representing complex, query-able groups of data. But most Apps will also require "globally unique" data of some kind. In Cevy ECS, we represent globally unique data using `Resources`.

Here are some examples of data that could be encoded as `Resources`:
    - Elapsed Time
    - Asset Collections (sounds, textures, meshes)
    - Renderers

## Tracking Time with Resources
Let's solve our App's "hello spam" problem by only printing "hello" once every two seconds. We'll do this by using the Time resource, which is automatically added to our App via add_plugins(DefaultPlugins).

For simplicity, remove the hello_world system from your App. This way we only need to adapt the greet_people system.

Resources are accessed in much the same way that we access components. You can access the Time resource in your system like this:
```cpp
void greet_people(Resource<Time> time, Query<Name> persons) {
    for (auto p : persons) {
        std::cout << "hello " << p.getName() << std::endl;
    }
}
```
The `delta` field on `Time` gives us the time that has passed since the last update. But in order to run our system once every two seconds, we must track the amount of time that has passed over a series of updates.
```cpp

struct GreetTime {
    Timer greet;
}

void greet_people(Resource<GreetTimer> timer, Resource<Time> time, Query<Name> persons) {
    if (timer.get().greet.tick(time.delta()).just_finished()) {
        for p : persons {
            std::cout << "hello " << p.getName() << std::endl;
        }
    }
}
```
Now all that's left is adding a `GreetTimer` Resource to our `HelloPlugin`.
```cpp
class HelloPlugin : public Plugin {
    public:
    void build(App &app) {
        app.init_component<Person>();
        app.insert_resource(GreetTimer{Timer(2, Timer::Repeating)});
        app.add_systems<core_stage::Startup>(add_people);
        app.add_systems<core_stage::Update>(hello_world, greet_people);
    }
};
```

[1]: https://github.com/Arthur-Aillet/Cevy "Cevy"
[2]: https://github.com/Arthur-Aillet/RType "Rtype"
[3]: https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools&rel=16 "VS"
