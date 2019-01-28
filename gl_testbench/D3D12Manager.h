#pragma once



class D3D12Manager {
private:
	void loadPipeline();
	void loadAssets();

public:
	D3D12Manager();
	~D3D12Manager();

	void initialize();
	void render();
	void update();
	void destroy();
};