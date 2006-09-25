// __BEGIN_LICENSE__
//
// Copyright (C) 2006 United States Government as represented by the
// Administrator of the National Aeronautics and Space Administration
// (NASA).  All Rights Reserved.
// 
// This software is distributed under the NASA Open Source Agreement
// (NOSA), version 1.3.  The NOSA has been approved by the Open Source
// Initiative.  See the file COPYING at the top of the distribution
// directory tree for the complete NOSA document.
// 
// THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY
// KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT
// LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO
// SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
// A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT
// THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT
// DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE.
//
// __END_LICENSE__

/// \file DiskImageResource.cc
/// 
/// An abstract base class referring to an image on disk.
/// 

#ifdef _MSC_VER
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4996)
#endif

#include <iostream>
#include <map>
#include <boost/algorithm/string.hpp>

#include <vw/FileIO/DiskImageResource.h>

#if defined(VW_HAVE_PKG_PNG) && VW_HAVE_PKG_PNG==1
#include <vw/FileIO/DiskImageResourcePNG.h>
#endif

namespace {
  typedef std::map<std::string,vw::DiskImageResource::construct_open_func> OpenMapType;
  typedef std::map<std::string,vw::DiskImageResource::construct_create_func> CreateMapType;
  OpenMapType *open_map = 0;
  CreateMapType *create_map = 0;
}

void vw::DiskImageResource::register_file_type( std::string const& extension,
                                                vw::DiskImageResource::construct_open_func open_func,
                                                vw::DiskImageResource::construct_create_func create_func )
{
  if( ! open_map ) open_map = new OpenMapType();
  if( ! create_map ) create_map = new CreateMapType();
  open_map->insert( std::make_pair( extension, open_func ) );
  create_map->insert( std::make_pair( extension, create_func ) );
}

static std::string file_extension( std::string const& filename ) {
  std::string::size_type dot = filename.find_last_of('.');
  if (dot != std::string::npos) {
    std::string extension = filename.substr( dot );
    boost::to_lower( extension );
    return extension;
  } else { 
    throw vw::IOErr() << "DiskImageResource: Cannot infer file format from filename with no file extension.";
  }
}

static void register_default_file_types() {
  static bool already = false;
  if( already ) return;
  already = true;
#if defined(VW_HAVE_PKG_PNG) && VW_HAVE_PKG_PNG==1
  vw::DiskImageResource::register_file_type( ".png", &vw::DiskImageResourcePNG::construct_open, &vw::DiskImageResourcePNG::construct_create );
#endif
}

vw::DiskImageResource* vw::DiskImageResource::open( std::string const& filename ) {
  register_default_file_types();
  if( open_map ) {
    OpenMapType::const_iterator i = open_map->find( file_extension( filename ) );
    if( i != open_map->end() ) return i->second( filename );
  }
  throw NoImplErr() << "Unsuppported file format: " << filename;
}

vw::DiskImageResource* vw::DiskImageResource::create( std::string const& filename, GenericImageFormat const& format ) {
  register_default_file_types();
  if( create_map ) {
    CreateMapType::const_iterator i = create_map->find( file_extension( filename ) );
    if( i != create_map->end() ) return i->second( filename, format );
  }
  throw NoImplErr() << "Unsuppported file format: " << filename;
}
