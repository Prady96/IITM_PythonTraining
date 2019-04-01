eight: thumbnailImageHeight,
    scaledX,
    scaledY,
  };
}

/**
 * @param {dataUrl} String Data URL of the original image.
 * @param {int} imageHeight Height in pixels of the original image.
 * @param {int} imageWidth Width in pixels of the original image.
 * @param {String} urlOrBlob 'blob' for a blob, otherwise data url.
 * @returns A promise that resolves to the data URL or blob of the thumbnail image, or null.
 */
function createThumbnail(dataUrl, imageWidth, imageHeight, urlOrBlob) {
  // There's cost associated with generating, transmitting, and storing
  // thumbnails, so we'll opt out if the image size is below a certain threshold
  const thumbnailThresholdFactor = 1.20;
  const thumbnailWidthThreshold = maxThumbnailWidth * thumbnailThresholdFactor;
  const thumbnailHeightThreshold = maxThumbnailHeight * thumbnailThresholdFactor;

  if (imageWidth <= thumbnailWidthThreshold &&
      imageHeight <= thumbnailHeightThreshold) {
    // Do not create a thumbnail.
    return Promise.resolve(null);
  }

  const thumbnailDimensions = getThumbnailDimensions(imageWidth, imageHeight);

  return new Promise((resolve, reject) => {
    const thumbnailImage = new Image();
    let srcWidth = imageWidth;
    let srcHeight = imageHeight;
    let destWidth, destHeight;

    thumbnailImage.onload = function() {
      destWidth = Math.round(srcWidth * maxResizeScaleFactor);
      destHeight = Math.round(srcHeight * maxResizeScaleFactor);
      if (destWidth <= thumbnailDimensions.scaledX || destHeight <= thumbnailDimensions.scaledY) {
        srcWidth = Math.round(srcWidth * (thumbnailDimensions.width / thumbnailDimensions.scaledX));
        srcHeight = Math.round(srcHeight * (thumbnailDimensions.height / thumbnailDimensions.scaledY));
        destWidth = thumbnailDimensions.width;
        destHeight = thumbnailDimensions.height;
      }

      const thumbnailCanvas = document.createElement("canvas");
      thumbnailCanvas.width = destWidth;
      thumbnailCanvas.height = destHeight;
      const ctx = thumbnailCanvas.getContext("2d");
      ctx.imageSmoothingEnabled = false;

      ctx.drawImage(
        thumbnailImage,
        0, 0, srcWidth, srcHeight,
        0, 0, destWidth, destHeight);

      if (thumbnailCanvas.width <= thumbnailDimensions.width ||
        thumbnailCanvas.height <= thumbnailDimensions.height) {
        if (urlOrBlob === "blob") {
          thumbnailCanvas.toBlob((blob) => {
            resolve(blob);
          });
        } else {
          resolve(thumbnailCanvas.toDataURL("image/png"));
        }
        return;
      }

      srcWidth = destWidth;
      srcHeight = destHeight;
      thumbnailImage.src = thumbnailCanvas.toDataURL();
    };
    thumbnailImage.src = dataUrl;
  });
}

function createThumbnailUrl(shot) {
  const image = shot.getClip(shot.clipNames()[0]).image;
  if (!image.url) {
    return Promise.resolve(null);
  }
  return createThumbnail(
    image.url, image.dimensions.x, image.dimensions.y, "dataurl");
}

function createThumbnailBlobFromPromise(shot, blobToUrlPromise) {
  return blobToUrlPromise.then(dataUrl => {
    const image = shot.getClip(shot.clipNames()[0]).image;
    return createThumbnail(
      dataUrl, image.dimensions.x, image.dimensions.y, "blob");
  });
}

if (typeof exports !== "undefined") {
  exports.getThumbnailDimensions = getThumbnailDimensions;
  exports.createThumbnailUrl = createThumbnailUrl;
  exports.createThumbnailBlobFromPromise = createThumbnailBlobFromPromise;
}

return exports;
})();
null;

PK