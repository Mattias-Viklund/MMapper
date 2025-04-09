#pragma once
// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2019 The MMapper Authors

#include "AbstractShaderProgram.h"

namespace Legacy {

struct NODISCARD AColorPlainShader final : public AbstractShaderProgram
{
public:
    using AbstractShaderProgram::AbstractShaderProgram;

    ~AColorPlainShader() final;

private:
    void virt_setUniforms(const glm::mat4 &mvp, const GLRenderState::Uniforms &uniforms) final
    {
        setColor("uColor", uniforms.color);
        setMatrix("uMVP", mvp);
    }
};

struct NODISCARD UColorPlainShader final : public AbstractShaderProgram
{
public:
    using AbstractShaderProgram::AbstractShaderProgram;

    ~UColorPlainShader() final;

private:
    void virt_setUniforms(const glm::mat4 &mvp, const GLRenderState::Uniforms &uniforms) final
    {
        setColor("uColor", uniforms.color);
        setMatrix("uMVP", mvp);
    }
};

struct NODISCARD AColorTexturedShader final : public AbstractShaderProgram
{
public:
    using AbstractShaderProgram::AbstractShaderProgram;

    ~AColorTexturedShader() final;

private:
    void virt_setUniforms(const glm::mat4 &mvp, const GLRenderState::Uniforms &uniforms) final
    {
        assert(uniforms.textures[0]);

        setColor("uColor", uniforms.color);
        setMatrix("uMVP", mvp);
        setTexture("uTexture", 0);
    }
};

struct NODISCARD UColorTexturedShader final : public AbstractShaderProgram
{
public:
    using AbstractShaderProgram::AbstractShaderProgram;

    ~UColorTexturedShader() final;

private:
    void virt_setUniforms(const glm::mat4 &mvp, const GLRenderState::Uniforms &uniforms) final
    {
        assert(uniforms.textures[0]);

        setColor("uColor", uniforms.color);
        setMatrix("uMVP", mvp);
        setTexture("uTexture", 0);
    }
};

struct NODISCARD WaterTexturedShader final : public AbstractShaderProgram
{
private:
    using Base = AbstractShaderProgram;

public:
    using Base::AbstractShaderProgram;
    ~WaterTexturedShader() final;

    // TODO: NO! ABSOLUTELY NOT
    std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::chrono::steady_clock::time_point startContinuous = std::chrono::high_resolution_clock::now();

    void updateTime() // TODO: WOW, THIS SEEMS 100% FINE. /s
    {
        if (!m_isBound)
            return;

        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> millis = now - start;
        float a = millis.count();

        if (a > 0.05f) {
            std::chrono::duration<float> totalMillis = now - startContinuous;
            float b = totalMillis.count();

            setFloat("fTime", b);
            start = now;
        }
    }

private:
    void virt_setUniforms(const glm::mat4 &mvp, const GLRenderState::Uniforms &uniforms) final
    {
        assert(uniforms.textures[0]);

        //setColor("uColor", uniforms.color);
        setMatrix("uMVP", mvp);
        //setTexture("uTexture", 0);

        //auto functions = Base::m_functions.lock();
        //Viewport res = deref(functions).getViewport();

        //updateTime();

        //setVec4("fResolution", glm::vec4(res.x, res.y, 0, 0));
    }
};

struct NODISCARD FontShader final : public AbstractShaderProgram
{
private:
    using Base = AbstractShaderProgram;

public:
    using Base::AbstractShaderProgram;

    ~FontShader() final;

private:
    void virt_setUniforms(const glm::mat4 &mvp, const GLRenderState::Uniforms &uniforms) final
    {
        assert(uniforms.textures[0]);
        auto functions = Base::m_functions.lock();

        setMatrix("uMVP3D", mvp);
        setTexture("uFontTexture", 0);
        setViewport("uPhysViewport", deref(functions).getPhysicalViewport());
    }
};

struct NODISCARD PointShader final : public AbstractShaderProgram
{
public:
    using AbstractShaderProgram::AbstractShaderProgram;

    ~PointShader() final;

private:
    void virt_setUniforms(const glm::mat4 &mvp, const GLRenderState::Uniforms &uniforms) final
    {
        setColor("uColor", uniforms.color);
        setMatrix("uMVP", mvp);
    }
};

/* owned by Functions */
struct NODISCARD ShaderPrograms final
{
private:
    Functions &m_functions;
    std::shared_ptr<AColorPlainShader> aColorShader;
    std::shared_ptr<UColorPlainShader> uColorShader;
    std::shared_ptr<AColorTexturedShader> aTexturedShader;
    std::shared_ptr<UColorTexturedShader> uTexturedShader;
    std::shared_ptr<WaterTexturedShader> waterTexturedShader;
    std::shared_ptr<FontShader> font;
    std::shared_ptr<PointShader> point;

public:
    explicit ShaderPrograms(Functions &functions)
        : m_functions{functions}
    {}

    DELETE_CTORS_AND_ASSIGN_OPS(ShaderPrograms);

private:
    NODISCARD Functions &getFunctions() { return m_functions; }

public:
    void resetAll()
    {
        aColorShader.reset();
        uColorShader.reset();
        aTexturedShader.reset();
        uTexturedShader.reset();
        waterTexturedShader.reset();
        font.reset();
        point.reset();
    }

public:
    // attribute color (aka "Colored")
    NODISCARD const std::shared_ptr<AColorPlainShader> &getPlainAColorShader();
    // uniform color (aka "Plain")
    NODISCARD const std::shared_ptr<UColorPlainShader> &getPlainUColorShader();
    // attribute color + textured (aka "ColoredTextured")
    NODISCARD const std::shared_ptr<AColorTexturedShader> &getTexturedAColorShader();
    // uniform color + textured (aka "Textured")
    NODISCARD const std::shared_ptr<UColorTexturedShader> &getTexturedUColorShader();
    NODISCARD const std::shared_ptr<WaterTexturedShader> &getTexturedWaterShader();
    NODISCARD const std::shared_ptr<FontShader> &getFontShader();
    NODISCARD const std::shared_ptr<PointShader> &getPointShader();
};

} // namespace Legacy
