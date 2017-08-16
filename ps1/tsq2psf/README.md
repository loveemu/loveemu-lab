Choro Q Wonderful! TSQ/TVB to PSF Converter
===========================================

TSQ2PSF converts TSQ/TVB sound files into PSF file.

It may also work for other Choro Q games. However, the result is not guaranteed.

Usage
-----

Syntax: `tsq2psf (options) outfile.psf`

Note: Please put tsq2psf.psflib to the directory where tsq2psf is installed

### Options

`--help`
  : Show help

`--standalone [TSQ path] [song index] [TVB path] `
  : Create a standalone PSF file

`--install-driver`
  : Install driver block for playback (for psflib creation)

`--tsq [TSQ path]`
  : Save TSQ file into output PSF file

`--tvb [TVB path]`
  : Save TVB file into output PSF file

`--song [song index]`
  : Specify song index for TSQ playback (often used with `--tsq` option)

`--lib [psflib name]`
  : Set the name to the `_lib` tag of output PSF file

`--psfby [name]`
  : Set the name to the `psfby` tag of output PSF file

### Example of Use

```
tsq2psf --standalone BGM_01.TSQ 1 BGM.TVB BGM_01.psf
```

```
tsq2psf --install-driver --tvb BGM.TVB BGM.psflib
tsq2psf --lib BGM.psflib --tsq BGM_01.TSQ --song 1 BGM_01.minipsf
```

```
tsq2psf --install-driver --tsq BGM_01.TSQ --tvb BGM.TVB BGM.psflib
tsq2psf --lib BGM.psflib --song 1 BGM_01.minipsf
```

You can edit reverb parameters saved in the driver block, by using PSF-o-Cycle.

Manual PSF Creation
-------------------

Follow those steps:

1. Open tsq2psf.psflib with PSFLab
2. Import the TSQ file to 0x800F0000
3. Import the TVB file to 0x8001D800
4. Write the song number to 0x800EF800
5. (Change EXE Settings to keep all the data - not necessary, it must be already good)
6. Save as PSF file

TSQ2PSF takes care of all those steps. Isn't it wonderful?
