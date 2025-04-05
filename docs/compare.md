# Bevy vs C++evy

## Component

Basic structure holding data, Ecs main building bloc

---
**Exemple Rust:**
```rust

#[derive(Component)]
struct Position {
    f64 x;
    f64 y;
}

```
**Exemple C++:**
```cpp

class Position {
    float x;
    float y;
};

```

## Entity

Group of Components

---
**Exemple Rust:**
```rust

commands.spawn((Camera, Position(3., 3.)));

```
**Exemple C++:**
```cpp

commands.spawn(Camera(), Position(3., 3.));

bundle camera = new bundle(new Camera, new Position(3, 3));
commands.spawn(camera);

//Implementation:

template<tyepname ...Args>
class bundle {
    public:
    bundle(Args...) { // Vargs
    }
}

```
## System

Function accessing one or
multiple component group

----
**Exemple Rust:**
```rust

fn hello_world() {
    println!("hello world!");
}

.add_systems(Update, hello_world)

//Advanced querys
fn debug_player_hp(
    query: Query<
      (&Health, Option<&PlayerName>),
      (With<Player>, Without<Enemy>)
    >,
)

```
**Exemple C++:**
```cpp


void hello_world()
{
    std::cout << "hello world!" << std::endl;
}

app.add_systems(hello_world)

void debug_player_hp(
        Query<Ref<Health>, Optional<Ref<const PlayerName>>, With<Player>, Without<Enemy>>& query
    )


```
*Features*

- Scheduler:
    - Custom set of steps
    - Custom ordering
- Conditional running
- Optional Querys
- Multithreaded
- Querys filters
    - With
    - Without
    - Or
- Extract Schedule as Depedencie graph

## App

Setup and manage Ecs,
Connect systems

---
**Exemple Rust:**
```rust

fn main() {
  app::new()
    .add_plugin(CameraPlugin)
    .add_systems(First, spawn_player)
    .init_resource(clock::new())
    .run();
}

```
**Exemple C++:**
```cpp

int main()
{
    App app;
    app.add_plugin<CameraPlugin>();
    app.add_systems(STAGE::First, spawn_player);
    app.init_resource<Clock>(/* initializer-list */); // for emplace-ctor
    app.init_resource(Clock());
    app.run();
}

```
## Resource
Global and Unique
data of some kind

---
**Exemple Rust:**
```rust

.init_resource(clock::new())

fn greet_people(time: Res<Time>, query: Query<&Name, With<Person>>) {
    for name in &query {
        println!("hello {}!", name.0);
    }
}

```
**Exemple C++:**
```cpp

.init_resources<Clock>(/* initializer-list */);

void greet_people(Resource<Time> time, Query<Name, With<Person>>& query)
{
    for (data : query) {
        std::cout << "hello " << std::get<0>(data) << "!" << std::endl;
    }
}

//Implementation:

template<typename T>
class Resource;

template<typename T>
using Res<T> = Resource<T>;
```
## Command

Interract with entity pool

---
**Exemple Rust:**
```rust

fn spawn_things( // System
    mut commands: Commands,
) {
    // manage resources
    commands.insert_resource(MyResource);
    commands.remove_resource::<MyResource>()
    commands.spawn(Velocity, Position::default());
    let id = commands.spawn(Velocity::new(0.0, -1.0));

    commands.entity(id)
      .insert(Position::default())
      .remove::<Velocity>();
}

```
**Exemple C++:**
```cpp

void spawn_things(Commands commands)
{
    commands.insert_resource<MyResource>();
    commands.remove_resource<MyResource>();
    commands.spawn(Position(), Velocity());

    Entity id = commands.spawn(Velocity(0., -1.));

    commands.entity(id).insert(Position()).remove<Velocity>();
}

```
## Plugins

Connect systems without Ecs management

Create an abstraction layer

---
**Exemple Rust:**
```rust

.add_plugins(HelloPlugin)
.add_plugins((HelloPlugin, GoodbyePlugin))
.add_plugins(HelloPluginGroup)

impl Plugin for HelloPlugin {
  fn build(&self, app: &mut App) {
    app.add_systems(Startup, spawn_camera)
      .add_systems(Update, move_camera)
      .add_systems(PostUpdate, destroy_camera)
      .add_event<CameraDestroyed>::new()
  }
}

```
**Exemple C++:**
```cpp

app.add_plugins<HelloPlugin>();
app.add_plugins<HelloPlugin, GoodbyePlugin>();
// app.add_plugins<HelloPluginGroup>()
/* -> not planned */

class HelloPlugin : public Plugin {
    void build(App& app) override
    {
        app.add_systems<core_stage::Startup>(spawn_camera);
        app.add_systems<core_stage::Update>(move_camera);
        app.add_systems<core_stage::PostUpdate>(destroy_camera);
        app.add_event(CameraDestroyed());
    }
}
```
## EventWriter

Send a message holding components

---
**Exemple Rust:**
```rust

fn event_trigger(
    mut cam_event: EventWriter<MyEvent>)
{
    cam_event.send(
      MyEvent{
        message: "Behold! a message...";
    })
}

```
**Exemple C++:**
```cpp

void event_trigger(EventWriter<MyEvent>& event)
{
    event.send(MyEvent {
        .message = "Behold! a message..."
    })
}

// this `.member = value` syntax is equivalent to rusts' `member: value` syntax
// it is for initalizing a struct's public members

```
## EventReader

System executed on resources and given component groups when signal is sent

---
**Exemple Rust:**
```rust

fn event_listener(mut events: EventReader<MyEvent>) {
    for my_event in events.read() {
        info!("{}", my_event.message);
    }
}

```
**Exemple C++:**
```cpp

void event_listener(EvenReader<MyEvent>& events)
{
    auto it = events.read();

    for (auto my_event: events) {
        std::cerr << my_event.message << std::endl;
    }
    for (const auto &my_event: events) {
        std::cerr << my_event.message << std::endl;
    }
}

```
## State

Define state of app

---
**Exemple Rust:**
```rust

#[derive(..., States)]
enum AppState {
    #[default]
    Menu,
    InGame,
}

App::new()
    .add_state::<AppState>()
    .add_systems(OnExit(AppState::Menu), ...)
    .add_systems(OnEnter(AppState::InGame), ...)
    .add_systems(
        Update, (...).run_if(in_state(AppState::InGame)),
    )

fn menu(mut next_state: ResMut<NextState<AppState>>) {
    if ... {
        next_state.set(AppState::InGame);
    }
}

```
**Exemple C++:**
```cpp

enum AppState {
    Menu,
    InGame,
};

App app;
app.add_state(AppState::Menu);
app.add_systems(OnExit(AppState::Menu), ...);
app.add_systems(OnEnter(AppState::Menu), ...);
// app.add_systems(Update, ..., run_if(?in_state(AppState::InGame)));
/* -> not planned */

void menu(Resource<NextState<AppState>>& next_state)
{
    if (...) {
        next_state.set(AppState::InGame);
    }
}


```

## GameEngine

App superset creating the default ECS App and adding
plugins to control bases, holding utilities for all games

---
**Resources:**

 - Asset Manager
 - Global Timer
 - Font Manager

**Components**

 - Position

**Default Stages:**

*At Start:*

 - PreStartup
 - Startup
 - PostStartup

*Each time possible:*

 - First
 - PreUpdate
 - StateTransition `// Run between each states transition`
 - RunFixedUpdateLoop `//This will run FixedUpdate zero to many times,
        based on how much time has elapsed.`
 - Update `// Default Stage when adding system`
 - PostUpdate
 - Last
