#ifndef METHOD_H
#define METHOD_H

#include <cfloat>
#include <string>
#include <vector>

#include "Camera.hpp"
#include "gl/GLHandler.hpp"
#include "utils.hpp"

class Method : public QObject
{
	Q_OBJECT
  public:
	Method(std::string const& shadersCommonName);
	Method(std::string const& vertexShaderPath,
	       std::string const& fragmentShaderPath);
	virtual BBox getDataBoundingBox() const   = 0;
	virtual void render(Camera const& camera) = 0;
	virtual ~Method()                         = default;

	GLShaderProgram shaderProgram;

  public slots:
	virtual std::string getName() const = 0;
	virtual void init(std::vector<float>& gazVertices,
	                  std::vector<float>& starsVertices,
	                  std::vector<float>& darkMatterVertices)
	    = 0;
	void init(std::string const& gazPath);
	void init(std::string const& gazPath, std::string const& starsPath);
	virtual void init(std::string const& gazPath, std::string const& starsPath,
	                  std::string const& darkMatterPath)
	    = 0;
	void resetAlpha();
	void setAlpha(float alpha);
	float getAlpha() const { return alpha; };
	virtual void setColors(QColor const& gasColor, QColor const& starsColor,
	                       QColor const& darkMatterColor)
	{
		this->gasColor        = gasColor;
		this->starsColor      = starsColor;
		this->darkMatterColor = darkMatterColor;
	};

	static void setDarkMatterEnabled(bool enabled) { showdm() = enabled; };
	static bool isDarkMatterEnabled() { return showdm(); };
	static void toggleDarkMatter() { showdm() = !showdm(); };
	// virtual void cleanUp() = 0;

  protected:
	static BBox globalBBox(std::vector<BBox> const& bboxes);

	float alpha = 0.011;
	static bool& showdm();

	QColor gasColor;
	QColor starsColor;
	QColor darkMatterColor;
};

#endif // DEFINE_H
