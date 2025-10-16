# SVG Studio Room Layouts

Example SVG files for loading studio room layouts into the acoustic wave simulation.

## Files

### `studio_simple.svg`
- **Complexity**: Beginner
- **Description**: Simple rectangular room divided by a vertical wall with a 1m door opening
- **Features**: Basic geometry, single dividing wall, 30cm wall thickness

### `studio_medium.svg`
- **Complexity**: Intermediate
- **Description**: L-shaped control room with circular isolation booth
- **Features**: Non-rectangular geometry, circular booth, acoustic panels, door opening

### `studio_complex.svg`
- **Complexity**: Advanced
- **Description**: Multi-room studio with two isolation booths, curved walls, and acoustic treatment
- **Features**: Curved diffusors, multiple circular booths, angled walls, bass traps, equipment rack

### `krasnodar_apartment_2br_detailed.svg`
- **Complexity**: Expert
- **Description**: Detailed Series-137 two-bedroom flat with wet core, entry gallery, open living/dining, and glazed loggia.
- **Features**: 320mm exterior walls, 180mm partitions, 0.9m door apertures (white fills), full furniture set (beds, wardrobes, sofa, kitchen, bath fixtures), balcony and soft absorbers for realistic reflections.
- **Scale**: Pixel-based 1280x880 canvas (1px = 8.6mm) matching the SMALL/XLARGE simulation grids.


## Usage

1. Launch the simulation
2. Press **L** key
3. Select an SVG file from this directory
4. The layout will be rasterized and loaded as obstacles

## SVG Format Specification

### Coordinate System
- **ViewBox**: Use `viewBox="0 0 20 10"` for a 20m x 10m room
- **Units**: 1 SVG unit = 1 meter
- **Grid mapping**: The simulation grid is 400x200 pixels (1 pixel = 5cm)
- **Automatic scaling**: SVG is automatically scaled to fit the simulation grid

### Wall Thickness
Use the `stroke-width` attribute to define wall thickness:
- `stroke-width="0.3"` = 30cm thick wall (typical interior wall)
- `stroke-width="0.25"` = 25cm thick wall (booth wall)
- `stroke-width="0.2"` = 20cm thick panel

### Supported SVG Elements

#### Rectangles
```xml
<rect x="0" y="0" width="20" height="10"
      fill="none" stroke="black" stroke-width="0.3"/>
```

#### Lines (Walls)
```xml
<line x1="12" y1="0" x2="12" y2="9"
      stroke="black" stroke-width="0.3"/>
```

#### Circles (Isolation Booths)
```xml
<circle cx="16.5" cy="3" r="2"
        fill="none" stroke="black" stroke-width="0.25"/>
```

#### Polygons
```xml
<polygon points="1,1 3,1 3,3 2,4 1,3"
         fill="black" stroke="none"/>
```

#### Paths (Complex Shapes, Curves)
```xml
<path d="M 10,2 Q 11.5,3 10,4"
      fill="none" stroke="black" stroke-width="0.2"/>
```

### Color Rules
- **Black/Dark** (`fill="black"` or `stroke="black"`): Becomes an obstacle (wall)
- **White/Light** (`fill="white"`): Becomes empty space (useful for doorways)
- **Transparent** (`fill="none"`): Uses stroke for walls
- **Labels/Text**: Ignored by rasterizer (can be any color)

### Design Tips

1. **Outer Walls**: Always include outer boundary
   ```xml
   <rect x="0" y="0" width="20" height="10"
         fill="none" stroke="black" stroke-width="0.3"/>
   ```

2. **Doorways**: Use gaps in lines or white-filled rectangles
   ```xml
   <!-- Leave gap in wall -->
   <line x1="0" y1="6" x2="8.5" y2="6" stroke="black" stroke-width="0.3"/>
   <line x1="10" y1="6" x2="20" y2="6" stroke="black" stroke-width="0.3"/>

   <!-- Or use white fill -->
   <rect x="9" y="6.7" width="1" height="0.6" fill="white"/>
   ```

3. **Curved Walls**: Use `<path>` with Bézier curves
   ```xml
   <path d="M 10,2 Q 11.5,3 10,4"
         fill="none" stroke="black" stroke-width="0.2"/>
   ```

4. **Circular Booths**: Use `<circle>` for isolation booths
   ```xml
   <circle cx="15" cy="3" r="2.5"
           fill="none" stroke="black" stroke-width="0.25"/>
   ```

5. **Acoustic Panels**: Use solid filled shapes
   ```xml
   <rect x="2" y="1.5" width="0.2" height="1.5"
         fill="black" stroke="none"/>
   ```

## Physical Scale Reference

| SVG Units | Meters | Pixels | Description |
|-----------|--------|--------|-------------|
| 0.05      | 5cm    | 1px    | Grid resolution |
| 0.2       | 20cm   | 4px    | Thin panel |
| 0.3       | 30cm   | 6px    | Standard wall |
| 1.0       | 1m     | 20px   | Door width |
| 2.0       | 2m     | 40px   | Small booth radius |
| 20 x 10   | 20m x 10m | 400x200px | Full room |

## Creating Your Own Layouts

### Using Inkscape
1. File → Document Properties → Custom Size: 20 x 10 units
2. View → Display Mode → Outline (to see strokes clearly)
3. Use Rectangle, Circle, and Pen tools
4. Set stroke width in Object → Fill and Stroke
5. Save As → Plain SVG (not Inkscape SVG)

### Using Adobe Illustrator
1. New Document → Width: 20, Height: 10, Units: Generic
2. Use Shape and Pen tools
3. Set Stroke Weight in Properties panel
4. File → Export → Export As → SVG
5. Options: Presentation Attributes (not Inline Styles)

### Using Code
```xml
<?xml version="1.0" encoding="UTF-8"?>
<svg viewBox="0 0 20 10" xmlns="http://www.w3.org/2000/svg">
  <!-- Your shapes here -->
</svg>
```

## Troubleshooting

### Walls appear too thin or thick
- Adjust `stroke-width` values
- Remember: 0.1 unit = 10cm = 2 pixels

### Layout doesn't fill the room
- Check your `viewBox` dimensions match "0 0 20 10"
- The SVG is automatically scaled to fit

### Curves look jagged
- Increase the simulation resolution (requires code change)
- Use smoother Bézier curves with fewer sharp angles

### Text appears as obstacles
- Make text elements very light colored (`fill="gray" opacity="0.3"`)
- Or place text outside viewBox boundaries

## Acoustic Design Considerations

1. **Parallel Walls**: Create standing waves (flutter echo)
   - Avoid perfectly parallel walls for better acoustics
   - Use splayed or angled walls

2. **Corners**: Accumulate bass energy
   - Add bass traps (solid triangular shapes in corners)
   - Round corners slightly to diffuse energy

3. **Isolation Booths**: Circular is better than rectangular
   - No parallel walls → less standing waves
   - More even sound distribution

4. **Diffusion**: Use curved or angled surfaces
   - Breaks up reflections
   - Creates more natural reverb

## Examples of Real Studios

The provided examples are inspired by common studio configurations:
- **Simple**: Basic rehearsal or podcast room
- **Medium**: Small recording studio with booth
- **Complex**: Professional multi-room facility

Feel free to modify these or create your own based on actual studio dimensions!
