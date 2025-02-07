#!/usr/bin/env python3
#
#  Generate documentation for RayZaler elements automatically, in Markdown
#  format with Doxygen extensions
#
#  Copyright (c) 2025 Gonzalo José Carracedo Carballal
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this program.  If not, see
#  <http:#www.gnu.org/licenses/>
#

import RayZaler as RZ
import numpy as np
import matplotlib.pyplot as plt
import pathlib
import sys
import PIL

APERTURE_COLOR = RZ.Vec3(0, 1, 0)
GRID_COLOR     = RZ.Vec3(0, 0, 1)
DOC_DIR        = pathlib.Path(__file__).parent.resolve()
RENDER_SIZE    = (1024, 1024)
THUMB_SIZE     = (256,   256)
DOC_SKIP       = ['RayBeamElement', 'Element', 'OpticalElement']
STL_EXAMPLE    = f'{DOC_DIR}/Utah_teapot_(solid)_smooth.stl' # https://makerworld.com/en/models/113685
DEFAULT_ELEM   = 'elem'
IMAGE_RFOLDER  = '../img'
IMAGE_FOLDER   = 'doxygen/img'
DOC_FOLDER     = 'elements'

def typeToName(typeId):
    if typeId == RZ.UndefinedValue:
        return "undefined"
    elif typeId == RZ.IntegerValue:
        return "integer"
    elif typeId == RZ.RealValue:
        return "real"
    elif typeId == RZ.BooleanValue:
        return "boolean"
    elif typeId == RZ.StringValue:
        return "string"
    
def valToString(propVal):
    typeId = propVal.type()
    if typeId == RZ.UndefinedValue:
        return "undefined"
    elif typeId == RZ.IntegerValue:
        return propVal.asInteger()
    elif typeId == RZ.RealValue:
        return propVal.asReal()
    elif typeId == RZ.BooleanValue:
        return propVal.asBool()
    elif typeId == RZ.StringValue:
        return propVal.asString()

class DocScene:
  def __init__(self, elemName: str):
    self._factoryName   = elemName
    self._code          = self.generateCodeForElement(elemName)
    self._topLevel      = RZ.TopLevelModel.fromString(self._code, [])
    self._element       = self._topLevel.lookupElement(DEFAULT_ELEM)
    self._world         = self._topLevel.world()
    self._gridThickness = 3
    self._axisZoom      = 5
    self._apThickness   = 5
    self._renderSize    = RENDER_SIZE

    p1 = RZ.Vec3()
    p2 = RZ.Vec3()
    self._element.boundingBox(p1, p2)
    self._boundingBox = (p1, p2)
    pass

  @staticmethod
  def generateCodeForElement(elemName):
    attrib = ''
    if elemName == 'Detector':
        attrib = 'flip = true'
    elif elemName == 'StlMesh':
        attrib = fr'file = "{STL_EXAMPLE}"'
    elif elemName == 'PhaseScreen':
        for i in range(28):
            if i > 0:
                attrib += ', '
            attrib += fr'Z{i} = {np.random.randn()}'
    modelCode = fr'{elemName} {DEFAULT_ELEM}({attrib});'
    return modelCode

  def calcBestGrid(
    self, 
    boundingBox: tuple[RZ.Vec3, RZ.Vec3],
    frame: RZ.ReferenceFrame = None,
    extra: float = 0):

    if frame is None:
      frame = self._world
    
    if boundingBox is None:
      boundingBox = self._boundingBox
    
    geometry = frame.toRelativeVec(boundingBox[1] - boundingBox[0])
    maxDim   = max(np.abs(geometry.x()), np.abs(geometry.y())) * (1 + extra)
    
    divOrd = 10**(np.floor(np.log10(maxDim / 10)))
    steps  = int(10**(np.ceil(np.log10(maxDim / divOrd))))
    
    return divOrd, steps

  def addGrid(
    self,
    modelRenderer: RZ.ModelRenderer,
    boundingBox: tuple[RZ.Vec3, RZ.Vec3] = None,
    frame: RZ.ReferenceFrame = None,
    extra: float = 0):

    if frame is None:
      frame = self._world
    
    step, divs = self.calcBestGrid(boundingBox, frame, extra) 
    grid = modelRenderer.addGrid("", frame)
    grid.grid.setGridStep(step)
    grid.grid.setGridDivs(divs)
    grid.grid.setColor(GRID_COLOR)
    grid.grid.setThickness(self._gridThickness)
    modelRenderer.setAxesZoom(self._axisZoom)
  
  def makeRenderer(self) -> RZ.ModelRenderer:
    renderer = RZ.ModelRenderer.fromOMModel(
      self._topLevel,
      self._renderSize[0],
      self._renderSize[1])
    renderer.model().setApertureThickness(self._apThickness)
    renderer.model().setApertureColor(APERTURE_COLOR)
    return renderer

  def renderScene(
    self,
    renderElements  = True,
    renderApertures = False,
    boundingBox: tuple[RZ.Vec3, RZ.Vec3] = None,
    frame: RZ.ReferenceFrame = None,
    extra: float = 0):
    if boundingBox is None:
      boundingBox = self._boundingBox

    renderer = self.makeRenderer()
    renderer.zoomToBox(self._world, boundingBox[0], boundingBox[1])
    renderer.setShowApertures(renderApertures)
    renderer.setShowElements(renderElements)
    self.addGrid(renderer, frame = frame, extra = extra)
    renderer.render()

    return renderer.image().copy()

  @staticmethod
  def blend(front: np.ndarray, back: np.ndarray, t: float = 0.25) -> np.ndarray:
    #mask = front[:, :, 3] / 255.
    img = np.zeros(front.shape, dtype = front.dtype)
    for i in range(4):
        img[:, :, i] = t * front[:, :, i] + (1 - t) * back[:, :, i]
    return img

  def renderSampleElement(self, opacity: float = 1) -> np.ndarray:
    apertureImg = self.renderScene(False, True, extra = 1e-2)
    solidImg    = self.renderScene(True, True, extra = 1e-2)
    return self.blend(apertureImg, solidImg, 1 - opacity)
  
  def renderPortFrame(
    self,
    frame: RZ.ReferenceFrame,
    opacity: float = 0.75) -> np.ndarray:
    p1 = RZ.Vec3()
    p2 = RZ.Vec3()
    self._element.boundingBox(p1, p2)
    bb = (p1, p2)

    # Expand box to fit in frame
    RZ.expandBox(p1, p2, frame.getCenter())
    gridOnly = self.renderScene(False, False, frame = frame, boundingBox = bb)
    solidImg = self.renderScene(True, False, frame = frame, boundingBox = bb)
    return self.blend(gridOnly, solidImg, 1 - opacity)

  def generateImages(self, folder = IMAGE_FOLDER):
    images = []

    # Generate sample image
    thumbPath = fr'{folder}/element-{self._factoryName}-thumbnail.png'
    imgPath   = fr'{folder}/element-{self._factoryName}.png'

    isOptical = self._element.hasProperty('optical')
    opacity = 0.75 if isOptical else 1
    
    img    = self.renderSampleElement(opacity)
    pilImg = PIL.Image.fromarray(img)
    pilImg.save(imgPath)
    pilImg.resize(THUMB_SIZE).save(thumbPath)

    images.append(imgPath)
    images.append(thumbPath)
    
    # Generate port images
    for port in self._element.ports():
        thumbPath   = fr'{folder}/port-{self._factoryName}-{port}-thumb.png'
        imgPath     = fr'{folder}/port-{self._factoryName}-{port}.png'
        portFrame   = self._element.getPortFrame(port)
        
        img         = self.renderPortFrame(portFrame)
        PIL.Image.fromarray(img).save(imgPath)
        plt.figure(figsize = (2, 2))
        plt.imshow(img)
        plt.title(port, fontfamily = 'Monospace')
        plt.axis('off')
        plt.savefig(thumbPath)
        plt.close()
        
        images.append(imgPath)
        images.append(thumbPath)

    return images

def generateImages():
  elements = RZ.Singleton.instance().elementFactories()
  for elName in elements:
    if elName in DOC_SKIP: continue
    
    print(fr'Generating images for {elName}')
    scene = DocScene(elName)
    files = scene.generateImages()
    for file in files:
      print(fr'  - {file}')
    
    print('')


def genDocForElement(elName: str):
  docPath   = fr'{DOC_FOLDER}/{elName}.md'
  thumbPath = fr'{IMAGE_RFOLDER}/element-{elName}-thumbnail.png'
  imgPath   = fr'{IMAGE_RFOLDER}/element-{elName}.png'

  factory   = RZ.Singleton.instance().lookupElementFactory(elName)
  meta      = factory.metaData()
  element   = factory.make(elName, RZ.WorldFrame("world"))
  isOptical = element.hasProperty('optical')

  # Header with image and brief description
  docText   = f'# {elName} {{#{elName}}}\n'
  docText  += f'{meta.description}.\n\n'
  if meta.parent is not None:
    parentName = meta.parent.name
    if parentName in DOC_SKIP:
      docText += f'<em><small>Specialization of <b>{parentName}</b></small></em>'
    else:
      docText += f'<em><small>Specialization of <b>@ref {parentName}</b></small></em>'

  docText  += f'\n\n'
  docText  += f'<center>'
  docText  += f'  <a href="{imgPath}"><img src="{thumbPath}" border="0" /></a>'
  docText  += f'</center>'
  docText  += f'<br /><br />\n'

  # Embed port enumeration
  ports       = element.ports()
  if len(ports) > 0:
    docText += f'### Reference frames\n'
    for portName in ports:
      thumbPath = fr'{IMAGE_RFOLDER}/port-{elName}-{portName}-thumb.png'
      imgPath   = fr'{IMAGE_RFOLDER}/port-{elName}-{portName}.png'
      docText  += f'<a href="{imgPath}"><img src="{thumbPath}" border="0" /></a> '

    docText += '\n'

  # Property table
  docText   += f'### Properties\n'
  docText   += '<table>\n'
  docText   += f'<tr><th>Name</th><th>Description</th><th>Default</th><th>Units</th></tr>\n'

  while meta is not None:
    currText = f'<tr><td colspan="4"><center><em><b>{meta.name}</b></em></center></td></tr>\n'
    count    = 0

    propList = list(meta.sortedProperties)
    if len(propList) == 0:
      propList = list(meta.properties)
        
    for prop in propList:
      if prop in ['optical']:
        continue
      
      propVal = meta.properties[prop]
      trueVal = element.get(prop)
      desc    = propVal.description()
      index   = desc.find('[')
      units   = ''
      
      if index >= 0:
        units = desc[index+1:].replace(']', '')
        desc  = desc[:index]
      
      count    += 1
      currText += f'<tr><td><div style="font-family: var(--font-family-monospace)">{prop}</div></td><td bgcolor="white">{desc}</td><td>{valToString(trueVal)}</td><td>{units}</td></tr>\n'
    
    # There is a non-zero amount of properties, document these
    if count > 0:
      docText += currText

    meta = meta.parent

  docText += '</table>\n'

  # For optical elements, give details on the optical path
  if isOptical:
    path = RZ.OpticalElement.fromElement(element).opticalPath()
    surfaces = path.surfaces()

    if len(surfaces) > 0:
      docText += '### Optical path\n'
      docText += '<div style="font-family: var(--font-family-monospace)">'
      docText += '  <ol>\n'
      for surf in surfaces:
        proc = path.getSurface(surf)
        docText += fr'<li><b>{surf}</b>({proc.processor.surfaceShape().name()}, {proc.processor.name()})</li>\n'

      docText += '  </ol>\n'
      docText += '</div>\n'

  # Save to file
  fp = open(docPath, "w")
  fp.write(docText)
  fp.close()
  print(fr'Documentation for {elName:20} written to {docPath}')

def genElementDocumentation():
  docPath   = fr'elementRef.md'

  tailText  = '@defgroup Elements'
  indexText = \
'''@page ElemRef Element referece

This version of RayZaler features the following built-in elements:
'''

  elements = RZ.Singleton.instance().elementFactories()
  for elName in elements:
    if elName in DOC_SKIP: continue
    
    genDocForElement(elName)

    indexText += f'- @subpage {elName}\n'    
    tailText  += '@addtogroup Elements\n'
    tailText  += f'''@copydoc {elName}
@{{
@}}
'''

  indexText += tailText
  fp = open(docPath, "w")
  fp.write(indexText)
  fp.close()

  print(fr'Index generated in {docPath}')

if len(sys.argv) != 2:
  print(fr'Usage: {sys.argv[0]} <images|documents>')
  sys.exit(1)

if sys.argv[1] == 'images':
  generateImages()
elif sys.argv[1] == 'documents':
  genElementDocumentation()
else:
  print(fr'{sys.argv[0]}: plase specify either images or documents')

