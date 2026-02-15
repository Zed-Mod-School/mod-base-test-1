#include "MarioRenderer2.h"

#include <vector>

#include "game/graphics/gfx.h"
#include "game/kernel/jak1/Mario1.h"
#include "third-party/stb_image/stb_image.h"

static GLuint overlayTex = 0;
static int overlayW = 0, overlayH = 0;

static const char* overlayVertSrc = R"GLSL(
#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
out vec2 fragUV;
uniform vec2 uPos;
uniform vec2 uSize;
uniform vec2 uScreen;
void main() {
    vec2 pos = uPos + inPos * uSize;
    vec2 ndc = (pos / uScreen) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    fragUV = inUV;
}
)GLSL";

static const char* overlayFragSrc = R"GLSL(
#version 330 core
in vec2 fragUV;
out vec4 FragColor;
uniform sampler2D uTex;
void main() {
    FragColor = texture(uTex, fragUV);
}
)GLSL";

static GLuint buildShader(const char* vs, const char* fs) {
    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    };
    GLuint v = compile(GL_VERTEX_SHADER, vs);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

MarioRenderer2::MarioRenderer2(GameVersion version) {
    // glGenVertexArrays(1, &m_vao);
    // glGenBuffers(1, &m_ubo);
    // glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    // glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // m_cube_vao = 0;
    // m_cube_vbo = 0;
    // m_cube_line_vbo = 0;
    // m_vbo_textured = 0;
    // m_vbo_untextured = 0;
    // m_texture = 0;
    // m_overlayVAO = 0;
    // m_overlayVBO = 0;
    // m_overlayShader = 0;

    // // Load Mario texture once (shared across all instances)
    // // uint8_t* tex_data = MarioManager::Get().GetSharedTexture();
    // // if (tex_data) {
    // //     glGenTextures(1, &m_texture);
    // //     glBindTexture(GL_TEXTURE_2D, m_texture);
    // //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
    // //                  SM64_TEXTURE_WIDTH, SM64_TEXTURE_HEIGHT, 0,
    // //                  GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    // //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // //     glBindTexture(GL_TEXTURE_2D, 0);
    // // }

    // // Overlay setup
    // m_overlayShader = buildShader(overlayVertSrc, overlayFragSrc);
    // glGenVertexArrays(1, &m_overlayVAO);
    // glGenBuffers(1, &m_overlayVBO);
    // glBindVertexArray(m_overlayVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, m_overlayVBO);

    // float quad[] = {
    //     0.f, 0.f, 0.f, 0.f,
    //     1.f, 0.f, 1.f, 0.f,
    //     1.f, 1.f, 1.f, 1.f,
    //     0.f, 1.f, 0.f, 1.f
    // };
    // glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    // glBindVertexArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
}

MarioRenderer2::~MarioRenderer2() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_ubo);
    if (m_cube_vao) glDeleteVertexArrays(1, &m_cube_vao);
    if (m_cube_vbo) glDeleteBuffers(1, &m_cube_vbo);
    if (m_cube_line_vbo) glDeleteBuffers(1, &m_cube_line_vbo);
    if (m_vbo_textured) glDeleteBuffers(1, &m_vbo_textured);
    if (m_vbo_untextured) glDeleteBuffers(1, &m_vbo_untextured);
    if (m_texture) glDeleteTextures(1, &m_texture);
    if (m_overlayVAO) glDeleteVertexArrays(1, &m_overlayVAO);
    if (m_overlayVBO) glDeleteBuffers(1, &m_overlayVBO);
    if (m_overlayShader) glDeleteProgram(m_overlayShader);
    if (overlayTex) glDeleteTextures(1, &overlayTex);
}

const float MARIO_SCALE_FACTOR = 4096.0f / 50.0f;

void MarioRenderer2::draw_surface_object(const SM64SurfaceObject& obj,
                                         const float rgba[4],
                                         SharedRenderState* render_state,
                                         bool outline) {
    struct Vertex { float pos[3]; float color[4]; };

    std::vector<Vertex> triangles, lines;
    const float* base = obj.transform.position;

    for (uint32_t i = 0; i < obj.surfaceCount; ++i) {
        const auto& tri = obj.surfaces[i];
        float v[3][3];
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                v[j][k] = ((float)tri.vertices[j][k] + base[k]) * MARIO_SCALE_FACTOR;

        for (int j = 0; j < 3; ++j)
            triangles.push_back({{v[j][0], v[j][1], v[j][2]}, {rgba[0], rgba[1], rgba[2], rgba[3]}});

        if (outline) {
            auto push = [&](int a, int b) {
                lines.push_back({{v[a][0], v[a][1], v[a][2]}, {0,0,0,1}});
                lines.push_back({{v[b][0], v[b][1], v[b][2]}, {0,0,0,1}});
            };
            push(0,1); push(1,2); push(2,0);
        }
    }

    // Use the same WORKING shader as MarioRenderer
    auto& sh = render_state->shaders[ShaderId::MARIO];
    sh.activate();
    GLuint sid = sh.id();

    glUniformMatrix4fv(glGetUniformLocation(sid, "camera"), 1, GL_FALSE, render_state->camera_matrix[0].data());
    glUniform4fv(glGetUniformLocation(sid, "hvdf_offset"), 1, render_state->camera_hvdf_off.data());
    glUniform4fv(glGetUniformLocation(sid, "camera_position"), 1, render_state->camera_pos.data());
    glUniform1f(glGetUniformLocation(sid, "fog_constant"), render_state->camera_fog.x());
    glUniform1f(glGetUniformLocation(sid, "fog_min"), render_state->camera_fog.y());
    glUniform1f(glGetUniformLocation(sid, "fog_max"), render_state->camera_fog.z());
    glUniform1i(glGetUniformLocation(sid, "version"), (GLint)render_state->version);
    glUniform1i(glGetUniformLocation(sid, "wireframe"), 999);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);

    if (m_cube_vao == 0) glGenVertexArrays(1, &m_cube_vao);
    glBindVertexArray(m_cube_vao);

    if (!triangles.empty()) {
        if (m_cube_vbo == 0) glGenBuffers(1, &m_cube_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
        glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(Vertex), triangles.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(4);
        glDisableVertexAttribArray(5);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)triangles.size());
    }

    if (outline && !lines.empty()) {
        if (m_cube_line_vbo == 0) glGenBuffers(1, &m_cube_line_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_cube_line_vbo);
        glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vertex), lines.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(4);

        glLineWidth(2.5f);
        glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());
    }

    glBindVertexArray(0);
}

void MarioRenderer2::render(SharedRenderState* render_state, ScopedProfilerNode& prof) {
    // Use the WORKING shader (same as MarioRenderer)
    auto& sh = render_state->shaders[ShaderId::MARIO];
    sh.activate();
    GLuint sid = sh.id();

    // Common uniforms (once)
    glUniform1i(glGetUniformLocation(sid, "wireframe"), 999);
    glUniformMatrix4fv(glGetUniformLocation(sid, "camera"), 1, GL_FALSE, render_state->camera_matrix[0].data());
    glUniform4fv(glGetUniformLocation(sid, "hvdf_offset"), 1, render_state->camera_hvdf_off.data());
    glUniform4fv(glGetUniformLocation(sid, "camera_position"), 1, render_state->camera_pos.data());
    glUniform1f(glGetUniformLocation(sid, "fog_constant"), render_state->camera_fog.x());
    glUniform1f(glGetUniformLocation(sid, "fog_min"), render_state->camera_fog.y());
    glUniform1f(glGetUniformLocation(sid, "fog_max"), render_state->camera_fog.z());
    glUniform1i(glGetUniformLocation(sid, "version"), (GLint)render_state->version);

    // Pipeline state (same as working version)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

    // ── Multi-Mario support ────────────────────────────────────────
    auto active = MarioManager::Get().GetActiveMarioIds();

    struct Vert { float pos[3]; float col[3]; float uv[2]; };
    std::vector<Vert> untex, tex;

    for (int id : active) {
        const auto& g = MarioManager::Get().GetMarioGeom(id);
        if (g.numTrianglesUsed == 0 || !g.position || !g.uv) continue;

        // Optional debug offset
        float ox = (id > 0) ? id * 300.0f : 0.0f;
        float oz = (id > 0) ? id * 200.0f : 0.0f;

        for (int i = 0; i < g.numTrianglesUsed * 3; ++i) {
            float u = g.uv[i*2+0], v = g.uv[i*2+1];
            if (u > 1.0f || u < -1.0f) {
                u /= 65535.0f;
                v /= 65535.0f;
            }

            bool textured = !(g.uv[i*2+0] == 1.0f && g.uv[i*2+1] == 1.0f);

            Vert vtx = {
                {g.position[i*3+0] * MARIO_SCALE_FACTOR + ox,
                 g.position[i*3+1] * MARIO_SCALE_FACTOR,
                 g.position[i*3+2] * MARIO_SCALE_FACTOR + oz},
                {g.color ? g.color[i*3+0] : 1.f,
                 g.color ? g.color[i*3+1] : 1.f,
                 g.color ? g.color[i*3+2] : 1.f},
                {u, v}
            };

            if (textured) tex.push_back(vtx);
            else          untex.push_back(vtx);
        }

        prof.add_tri(g.numTrianglesUsed);
    }

    // ── Draw ───────────────────────────────────────────────────────
    auto draw = [&](const auto& verts, GLuint& vbo, bool textured) {
        if (verts.empty()) return;
        if (vbo == 0) glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vert), verts.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert,pos));
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert,col));
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert,uv));

        if (textured && m_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glUniform1i(glGetUniformLocation(sid, "u_texture"), 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
    };

    draw(untex, m_vbo_untextured, false);
    draw(tex,   m_vbo_textured,   true);

    glBindVertexArray(0);

    // ── Cubes ──────────────────────────────────────────────────────
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_TRUE);

    for (int i = 0; i < numCubes; ++i) {
        float yellow[4] = {1.0f, 1.0f, 0.0f, 0.4f};
        draw_surface_object(spawnedCubes[i].surfaceObj, yellow, render_state, true);
    }

    // ── Overlay ────────────────────────────────────────────────────
    if (overlayTex == 0) {
        int n; unsigned char* data = stbi_load("overlay.png", &overlayW, &overlayH, &n, 4);
        if (data) {
            glGenTextures(1, &overlayTex);
            glBindTexture(GL_TEXTURE_2D, overlayTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, overlayW, overlayH, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(data);
        }
    }

    if (overlayTex && m_overlayShader) {
        glUseProgram(m_overlayShader);
        glBindVertexArray(m_overlayVAO);

        glUniform2f(glGetUniformLocation(m_overlayShader, "uPos"), 100.f, 100.f);
        glUniform2f(glGetUniformLocation(m_overlayShader, "uSize"), (float)overlayW, (float)overlayH);
        glUniform2f(glGetUniformLocation(m_overlayShader, "uScreen"),
                    (float)render_state->render_fb_w, (float)render_state->render_fb_h);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, overlayTex);
        glUniform1i(glGetUniformLocation(m_overlayShader, "uTex"), 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Restore state
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    prof.add_draw_call();
}