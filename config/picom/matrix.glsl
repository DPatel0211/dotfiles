#version 330
in vec2 texcoord;
uniform sampler2D tex;
vec4 default_post_processing(vec4 c);
ivec2 window_size = textureSize(tex, 0);
vec4 window_shader() {
    vec4 c = texelFetch(tex, ivec2(texcoord), 0);
    c = default_post_processing(c);
    float time = c.w * 1.25;
    
    if (time < 0.01) return vec4(0);
    
    // Center of the notification window (not just 300px assumption)
    vec2 center = vec2(window_size.x / 2.0, window_size.y / 2.0);
    vec2 uv = texcoord - center;
    float dist = length(uv);
    
    // Start portal smaller to hide content initially, expand beyond window size
    // Start at 0, expand to cover diagonal of window
    float max_radius = length(vec2(window_size)) * 0.7;
    float portal_radius = time * max_radius;
    
    // Delay the reveal slightly to let the border appear first
    float delayed_time = max(0.0, (time - 0.1) / 0.9); // 10% delay
    float delayed_radius = delayed_time * max_radius;
    
    // Simple reveal with soft edge using delayed radius
    float reveal = smoothstep(delayed_radius + 10.0, delayed_radius - 10.0, dist);
    
    vec4 final_c = c;
    final_c.w *= reveal;
    
    // Edge glow during opening (using original time for smooth glow)
    float edge_glow = exp(-abs(dist - portal_radius) * 0.05) * (1.0 - time);
    final_c.rgb *= 1.0 + edge_glow * 0.3;
    
    // Subtle brightness boost that increases as animation completes
    float brightness_boost = time * 0.15;
    final_c.rgb *= 1.0 + brightness_boost;
    
    return default_post_processing(final_c);
}
