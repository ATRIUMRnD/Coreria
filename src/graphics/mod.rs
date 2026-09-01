/*!
# Graphics Subsystem

OpenGL-based rendering system integrated with Bevy's rendering pipeline.
Handles window management, OpenGL context, and custom rendering features.
*/

use bevy::prelude::*;
use log::{info, debug};

pub mod opengl;
pub mod shaders;
pub mod textures;

/// Graphics plugin for the engine
pub struct GraphicsPlugin;

impl Plugin for GraphicsPlugin {
    fn build(&self, app: &mut App) {
        info!("🎨 Initializing Graphics Plugin");
        
        app
            // Add graphics resources
            .insert_resource(GraphicsSettings::default())
            .insert_resource(RenderStats::default())
            
            // Add graphics systems
            .add_systems(Startup, graphics_startup)
            .add_systems(Update, (
                update_render_stats,
                handle_graphics_events,
            ));
    }
}

/// Graphics configuration
#[derive(Resource, Debug, Clone)]
pub struct GraphicsSettings {
    pub render_scale: f32,
    pub shadow_quality: ShadowQuality,
    pub anti_aliasing: AntiAliasing,
    pub texture_filtering: TextureFiltering,
    pub max_fps: Option<u32>,
}

impl Default for GraphicsSettings {
    fn default() -> Self {
        Self {
            render_scale: 1.0,
            shadow_quality: ShadowQuality::High,
            anti_aliasing: AntiAliasing::MSAA4x,
            texture_filtering: TextureFiltering::Linear,
            max_fps: Some(60),
        }
    }
}

/// Shadow quality settings
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ShadowQuality {
    Disabled,
    Low,
    Medium, 
    High,
    Ultra,
}

/// Anti-aliasing options
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AntiAliasing {
    None,
    MSAA2x,
    MSAA4x,
    MSAA8x,
    FXAA,
    TAA,
}

/// Texture filtering modes
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TextureFiltering {
    Nearest,
    Linear,
    Anisotropic2x,
    Anisotropic4x,
    Anisotropic8x,
    Anisotropic16x,
}

/// Render performance statistics
#[derive(Resource, Debug, Default)]
pub struct RenderStats {
    pub triangles_rendered: u64,
    pub draw_calls: u32,
    pub texture_switches: u32,
    pub shader_switches: u32,
    pub render_time_ms: f64,
}

/// Graphics startup system
fn graphics_startup(
    mut commands: Commands,
    settings: Res<GraphicsSettings>,
) {
    info!("🎬 Graphics system startup");
    debug!("🖼️  Render scale: {}", settings.render_scale);
    debug!("🌑 Shadow quality: {:?}", settings.shadow_quality);
    debug!("🔍 Anti-aliasing: {:?}", settings.anti_aliasing);
    
    // Spawn a camera for the engine
    commands.spawn((
        Camera3dBundle {
            transform: Transform::from_xyz(0.0, 5.0, 10.0)
                .looking_at(Vec3::ZERO, Vec3::Y),
            ..default()
        },
        Name::new("CoreiaMainCamera"),
    ));
    
    // Add some basic lighting
    commands.spawn((
        DirectionalLightBundle {
            directional_light: DirectionalLight {
                illuminance: 10000.0,
                shadows_enabled: true,
                ..default()
            },
            transform: Transform::from_rotation(
                Quat::from_euler(EulerRot::XYZ, -0.5, -0.5, 0.0)
            ),
            ..default()
        },
        Name::new("CoreiaMainLight"),
    ));
}

/// Update rendering statistics
fn update_render_stats(
    time: Res<Time>,
    mut stats: ResMut<RenderStats>,
) {
    // Reset per-frame stats
    stats.triangles_rendered = 0;
    stats.draw_calls = 0;
    stats.texture_switches = 0;
    stats.shader_switches = 0;
    
    // This would be populated by the actual rendering systems
    // For now, simulate some basic metrics
    stats.render_time_ms = time.delta().as_secs_f64() * 1000.0;
}

/// Handle graphics-related events
fn handle_graphics_events(
    keyboard_input: Res<Input<KeyCode>>,
    mut settings: ResMut<GraphicsSettings>,
) {
    // Toggle wireframe mode with F1
    if keyboard_input.just_pressed(KeyCode::F1) {
        debug!("🔧 F1 pressed - wireframe toggle (not implemented)");
    }
    
    // Cycle anti-aliasing with F2
    if keyboard_input.just_pressed(KeyCode::F2) {
        settings.anti_aliasing = match settings.anti_aliasing {
            AntiAliasing::None => AntiAliasing::MSAA2x,
            AntiAliasing::MSAA2x => AntiAliasing::MSAA4x,
            AntiAliasing::MSAA4x => AntiAliasing::MSAA8x,
            AntiAliasing::MSAA8x => AntiAliasing::None,
            _ => AntiAliasing::MSAA4x,
        };
        info!("🔍 Anti-aliasing changed to: {:?}", settings.anti_aliasing);
    }
}