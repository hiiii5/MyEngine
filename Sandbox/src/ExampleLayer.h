#pragma once

#include "MyEngine.h"

class ExampleLayer : public MyEngine::Layer {
public:
  ExampleLayer();
  virtual ~ExampleLayer() = default;

  virtual void OnAttach() override;
  virtual void OnDetach() override;

  virtual void OnUpdate(MyEngine::Timestep ts) override;
  virtual void OnImGuiRender() override;

  virtual void OnEvent(MyEngine::Event &e, void *pData) override;
};
