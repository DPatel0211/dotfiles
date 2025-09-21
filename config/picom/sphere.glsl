#version 330
in vec2 texcoord;
uniform sampler2D tex;
ivec2 window_size = textureSize(tex, 0); 
ivec2 window_center = ivec2(window_size.x/2, window_size.y/2);

vec4 default_post_processing(vec4 c);

float max_opacity = 0.8;
float opacity_threshold(float opacity) {
    return opacity >= max_opacity ? 1.0 : min(1, opacity/max_opacity);
}

vec4 anim(float progress) {
    vec4 c = texelFetch(tex, ivec2(texcoord), 0);
    if (progress <= 0.001) { c.a = 0.0; return c; }
    if (progress >= 0.999) return c;

    vec2 p_centered = texcoord - vec2(window_center);

    float max_coverage_radius = length(vec2(window_size) * 0.5) * 1.1;
    float eased = sqrt(progress); // ease-out
    float base_radius = eased * max_coverage_radius;

    float angle = atan(p_centered.y, p_centered.x);
    float wobble = sin(angle * 5.0 + progress * 6.0) * base_radius * 0.08;

    // gentle breathing pulse
    wobble += base_radius * 0.03 * sin(progress * 3.1415);

    float dist = length(p_centered) - (base_radius + wobble);
    float mask = 1.0 - smoothstep(0.0, 30.0, dist);

    c.a *= mask;
    return c;
}

vec4 window_shader() {
    vec4 c = texelFetch(tex, ivec2(texcoord), 0);
    c = default_post_processing(c);
    float opacity = opacity_threshold(c.w);
    if (opacity == 0.0) return c;
    vec4 anim_c = anim(opacity);
    if (anim_c.w < max_opacity) return vec4(0);
    return default_post_processing(anim_c);
}
